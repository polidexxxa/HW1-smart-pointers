#ifndef _SHRDPTR_HPP_
#define _SHRDPTR_HPP_

#include <cstddef>
#include <type_traits>
#include <utility>
#include "UnqPtr.hpp"

template <typename T>
class ShrdPtr {
private:
    T* ptr = nullptr;
    size_t* refCount = nullptr;

    template <typename U>
    friend class ShrdPtr;

public:
    ShrdPtr() noexcept : ptr(nullptr), refCount(nullptr) {}

    explicit ShrdPtr(T* rawPtr) 
        : ptr(rawPtr), refCount(rawPtr ? new size_t(1) : nullptr) {}

    explicit ShrdPtr(UnqPtr<T>&& unq) 
        : ptr(unq.Release()), refCount(ptr ? new size_t(1) : nullptr) {}
        
    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    explicit ShrdPtr(UnqPtr<U>&& unq) 
        : ptr(unq.Release()), refCount(ptr ? new size_t(1) : nullptr) {}
    
    ~ShrdPtr() {
        if (refCount != nullptr) {
            --(*refCount);
            if (*refCount == 0) {
                delete ptr;
                delete refCount;
            }
            ptr = nullptr;
            refCount = nullptr;
        }
    }

    ShrdPtr(const ShrdPtr& other) noexcept : ptr(other.ptr), refCount(other.refCount) {
        if (refCount != nullptr) {
            ++(*refCount);
        }
    }

    ShrdPtr(ShrdPtr&& other) noexcept : ptr(other.ptr), refCount(other.refCount) {
        other.ptr = nullptr;
        other.refCount = nullptr;
    }

    ShrdPtr& operator=(ShrdPtr other) noexcept {
        Swap(other);
        return *this;
    }

    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    ShrdPtr(const ShrdPtr<U>& other) noexcept : ptr(other.ptr), refCount(other.refCount) {
        if (refCount != nullptr) {
            ++(*refCount);
        }
    }

    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    ShrdPtr(ShrdPtr<U>&& other) noexcept : ptr(other.ptr), refCount(other.refCount) {
        other.ptr = nullptr;
        other.refCount = nullptr;
    }

    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    ShrdPtr& operator=(ShrdPtr<U> other) noexcept {
        ShrdPtr temp(std::move(other));
        Swap(temp);
        return *this;
    }

    T* Get() const noexcept {
        return ptr;
    }

    T& operator*() const noexcept {
        return *ptr;
    }

    T* operator->() const noexcept {
        return ptr;
    }

    explicit operator bool() const noexcept {
        return ptr != nullptr;
    }

    size_t UseCount() const noexcept {
        return refCount != nullptr ? *refCount : 0;
    }

    void Reset(T* rawPtr = nullptr) {
        ShrdPtr temp(rawPtr);
        Swap(temp);
    }

    void Swap(ShrdPtr& other) noexcept {
        T* tempPtr = ptr;
        ptr = other.ptr;
        other.ptr = tempPtr;

        size_t* tempRefCount = refCount;
        refCount = other.refCount;
        other.refCount = tempRefCount;
    }

};

//ARRAYS 
template <typename T>
class ShrdPtr<T[]> {
private:
    T* ptr = nullptr;
    size_t* refCount = nullptr;

    template <typename U>
    friend class ShrdPtr;

public:
    ShrdPtr() noexcept : ptr(nullptr), refCount(nullptr) {}

    explicit ShrdPtr(T* rawArray) 
        : ptr(rawArray), refCount(rawArray ? new size_t(1) : nullptr) {}

    explicit ShrdPtr(UnqPtr<T[]>&& unq) 
        : ptr(unq.Release()), refCount(ptr ? new size_t(1) : nullptr) {}

    ~ShrdPtr() {
        if (refCount != nullptr) {
            --(*refCount);
            if (*refCount == 0) {
                delete[] ptr;
                delete refCount;
            }
            ptr = nullptr;
            refCount = nullptr;
        }
    }

    ShrdPtr(const ShrdPtr& other) noexcept : ptr(other.ptr), refCount(other.refCount) {
        if (refCount != nullptr) {
            ++(*refCount);
        }
    }

    ShrdPtr(ShrdPtr&& other) noexcept : ptr(other.ptr), refCount(other.refCount) {
        other.ptr = nullptr;
        other.refCount = nullptr;
    }

    ShrdPtr& operator=(ShrdPtr other) noexcept {
        Swap(other);
        return *this;
    }

    T& operator[](size_t index) const noexcept {
        return ptr[index];
    }

    T* Get() const noexcept { 
        return ptr; 
    }
    
    explicit operator bool() const noexcept { 
        return ptr != nullptr; 
    }

    size_t UseCount() const noexcept {
        return refCount != nullptr ? *refCount : 0;
    }

    void Reset(T* rawArray = nullptr) {
        ShrdPtr temp(rawArray);
        Swap(temp);
    }

    void Swap(ShrdPtr& other) noexcept {
        T* tempPtr = ptr;
        ptr = other.ptr;
        other.ptr = tempPtr;

        size_t* tempRefCount = refCount;
        refCount = other.refCount;
        other.refCount = tempRefCount;
    }
};


template <typename T, typename... Args>
std::enable_if_t<!std::is_array_v<T>, ShrdPtr<T>>
MakeShrd(Args&&... args) {
    return ShrdPtr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
std::enable_if_t<std::is_array_v<T>, ShrdPtr<T>>
MakeShrd(size_t size) {
    using ElementType = std::remove_extent_t<T>;
    return ShrdPtr<T>(new ElementType[size]());
}

#endif //_SHRDPTR_HPP_