#ifndef _UNQPTR_HPP_
#define _UNQPTR_HPP_

#include <type_traits>
#include <utility>

template <typename T>
class UnqPtr {
private:
    T* ptr = nullptr;

    template <typename U>
    friend class UnqPtr;

public:
    UnqPtr() noexcept : ptr(nullptr) {}  
    explicit UnqPtr(T* rawPtr) noexcept : ptr(rawPtr) {}

    ~UnqPtr() {
        delete ptr;
    }

    UnqPtr(const UnqPtr&) = delete;
    UnqPtr& operator=(const UnqPtr&) = delete;

    UnqPtr(UnqPtr&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }

    UnqPtr& operator=(UnqPtr&& other) noexcept {
        if (this != &other) {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    UnqPtr(UnqPtr<U>&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }

    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
    UnqPtr& operator=(UnqPtr<U>&& other) noexcept {
        delete ptr;
        ptr = other.ptr;
        other.ptr = nullptr;
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

    T* Release() noexcept {
        T* temp = ptr;
        ptr = nullptr;
        return temp;
    }

    void Reset(T* rawPtr = nullptr) noexcept {
        T* oldPtr = ptr;
        ptr = rawPtr;
        delete oldPtr;
    }

    void Swap(UnqPtr& other) noexcept {
        T* tempPtr = ptr;
        ptr = other.ptr;
        other.ptr = tempPtr;
    }

};

//МАССИВЫ
template <typename T>
class UnqPtr<T[]> {
private:
    T* ptr = nullptr;

    template <typename U>
    friend class UnqPtr;

public:
    UnqPtr() noexcept : ptr(nullptr) {}
    explicit UnqPtr(T* rawArray) noexcept : ptr(rawArray) {}

    ~UnqPtr() {
        delete[] ptr;
    }

    UnqPtr(const UnqPtr&) = delete;
    UnqPtr& operator=(const UnqPtr&) = delete;

    UnqPtr(UnqPtr&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }

    UnqPtr& operator=(UnqPtr&& other) noexcept {
        if (this != &other) {
            delete[] ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
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

    T* Release() noexcept {
        T* temp = ptr;
        ptr = nullptr;
        return temp;
    }

    void Reset(T* rawArray = nullptr) noexcept {
        T* oldPtr = ptr;
        ptr = rawArray;
        delete[] oldPtr;
    }

    void Swap(UnqPtr& other) noexcept {
        T* temp = ptr;
        ptr = other.ptr;
        other.ptr = temp;
    }
};


template <typename T, typename... Args>
std::enable_if_t<!std::is_array_v<T>, UnqPtr<T>>
MakeUnq(Args&&... args) {
    return UnqPtr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
std::enable_if_t<std::is_array_v<T>, UnqPtr<T>>
MakeUnq(size_t size) {
    using ElementType = std::remove_extent_t<T>;
    return UnqPtr<T>(new ElementType[size]());
}

#endif //_UNQPTR_HPP_