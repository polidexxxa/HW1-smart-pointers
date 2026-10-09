#include <gtest/gtest.h>
#include <utility>
#include "ShrdPtr.hpp"
#include "UnqPtr.hpp"

struct ShrdTracker {
    static int aliveCount;
    int data = 0;

    ShrdTracker() : data(0) { ++aliveCount; }
    explicit ShrdTracker(int d) : data(d) { ++aliveCount; }
    ShrdTracker(const ShrdTracker& other) : data(other.data) { ++aliveCount; }
    ShrdTracker(ShrdTracker&& other) noexcept : data(other.data) { ++aliveCount; }
    ~ShrdTracker() { --aliveCount; }
};
int ShrdTracker::aliveCount = 0;

struct ShrdBase {
    int x = 1;
    virtual ~ShrdBase() = default;
    virtual int Get() const { return x; }
};

struct ShrdDerived : public ShrdBase {
    int y = 2;
    int Get() const override { return x + y; }
};

class ShrdPtrFixture : public ::testing::Test {
protected:
    void SetUp() override {
        ShrdTracker::aliveCount = 0;
    }
    void TearDown() override {
        EXPECT_EQ(ShrdTracker::aliveCount, 0);
    }
};

TEST_F(ShrdPtrFixture, ReferenceCountOnCopy) {
    {
        ShrdPtr<ShrdTracker> s1 = MakeShrd<ShrdTracker>(10);
        EXPECT_EQ(s1.UseCount(), 1);
        EXPECT_EQ(ShrdTracker::aliveCount, 1);

        {
            ShrdPtr<ShrdTracker> s2 = s1;
            EXPECT_EQ(s1.UseCount(), 2);
            EXPECT_EQ(s2.UseCount(), 2);
            EXPECT_EQ(ShrdTracker::aliveCount, 1);

            ShrdPtr<ShrdTracker> s3;
            s3 = s2;
            EXPECT_EQ(s1.UseCount(), 3);
            EXPECT_EQ(s3.UseCount(), 3);
        }

        EXPECT_EQ(s1.UseCount(), 1);
        EXPECT_EQ(ShrdTracker::aliveCount, 1);
    }
    EXPECT_EQ(ShrdTracker::aliveCount, 0);
}

TEST_F(ShrdPtrFixture, MoveConstruction) {
    auto s1 = MakeShrd<ShrdTracker>(25);
    EXPECT_EQ(s1.UseCount(), 1);

    ShrdPtr<ShrdTracker> s2 = std::move(s1);
    EXPECT_FALSE(s1);
    EXPECT_EQ(s1.UseCount(), 0);
    EXPECT_EQ(s2.UseCount(), 1);
    EXPECT_EQ(ShrdTracker::aliveCount, 1);
}

TEST_F(ShrdPtrFixture, ConstructFromUnqPtr) {
    UnqPtr<ShrdTracker> u = MakeUnq<ShrdTracker>(40);
    EXPECT_EQ(ShrdTracker::aliveCount, 1);

    ShrdPtr<ShrdTracker> s(std::move(u));
    EXPECT_FALSE(u);
    EXPECT_EQ(s.UseCount(), 1);
    EXPECT_EQ(s->data, 40);
    EXPECT_EQ(ShrdTracker::aliveCount, 1);
}

TEST_F(ShrdPtrFixture, PolymorphicSubtyping) {
    ShrdPtr<ShrdDerived> childPtr = MakeShrd<ShrdDerived>();
    EXPECT_EQ(childPtr.UseCount(), 1);

    ShrdPtr<ShrdBase> parentPtr = childPtr;
    EXPECT_EQ(childPtr.UseCount(), 2);
    EXPECT_EQ(parentPtr.UseCount(), 2);
    EXPECT_EQ(parentPtr->Get(), 3);

    UnqPtr<ShrdDerived> unqChild = MakeUnq<ShrdDerived>();
    ShrdPtr<ShrdBase> parentFromUnq(std::move(unqChild));
    EXPECT_FALSE(unqChild);
    EXPECT_EQ(parentFromUnq.UseCount(), 1);
    EXPECT_EQ(parentFromUnq->Get(), 3);
}

TEST_F(ShrdPtrFixture, ArraySpecialization) {
    {
        auto arr1 = MakeShrd<ShrdTracker[]>(4);
        EXPECT_EQ(arr1.UseCount(), 1);
        EXPECT_EQ(ShrdTracker::aliveCount, 4);

        {
            auto arr2 = arr1;
            EXPECT_EQ(arr1.UseCount(), 2);
            EXPECT_EQ(arr2.UseCount(), 2);
            arr2[1].data = 99;
            EXPECT_EQ(arr1[1].data, 99);
        }

        EXPECT_EQ(arr1.UseCount(), 1);
        EXPECT_EQ(ShrdTracker::aliveCount, 4);
    }
    EXPECT_EQ(ShrdTracker::aliveCount, 0);
}

TEST_F(ShrdPtrFixture, ConstructArrayFromUnqPtr) {
    const size_t kSize = 4;
    auto unqArr = MakeUnq<ShrdTracker[]>(kSize);
    for (size_t i = 0; i < kSize; ++i) {
        unqArr[i].data = static_cast<int>((i + 1) * 11);
    }
    EXPECT_EQ(ShrdTracker::aliveCount, 4);

    ShrdPtr<ShrdTracker[]> shrdArr(std::move(unqArr));
    EXPECT_FALSE(unqArr);
    EXPECT_EQ(unqArr.Get(), nullptr);
    EXPECT_TRUE(shrdArr);
    EXPECT_EQ(shrdArr.UseCount(), 1);
    EXPECT_EQ(ShrdTracker::aliveCount, 4);

    EXPECT_EQ(shrdArr[0].data, 11);
    EXPECT_EQ(shrdArr[3].data, 44);

    {
        auto secondOwner = shrdArr;
        EXPECT_EQ(shrdArr.UseCount(), 2);
        EXPECT_EQ(secondOwner.UseCount(), 2);
    }
    EXPECT_EQ(shrdArr.UseCount(), 1);
    EXPECT_EQ(ShrdTracker::aliveCount, 4);
}

TEST_F(ShrdPtrFixture, ArrayMoveAssignment) {
    auto s1 = MakeShrd<ShrdTracker[]>(3);
    s1[0].data = 100;
    EXPECT_EQ(s1.UseCount(), 1);
    EXPECT_EQ(ShrdTracker::aliveCount, 3);

    ShrdPtr<ShrdTracker[]> s2;
    s2 = std::move(s1);

    EXPECT_FALSE(s1);
    EXPECT_EQ(s1.UseCount(), 0);
    EXPECT_TRUE(s2);
    EXPECT_EQ(s2.UseCount(), 1);
    EXPECT_EQ(s2[0].data, 100);
    EXPECT_EQ(ShrdTracker::aliveCount, 3);
}

TEST_F(ShrdPtrFixture, ZeroLengthArray) {
    {
        auto zeroArr = MakeShrd<ShrdTracker[]>(0);
        EXPECT_TRUE(zeroArr);
        EXPECT_EQ(zeroArr.UseCount(), 1);
        EXPECT_EQ(ShrdTracker::aliveCount, 0); 
    }
    EXPECT_EQ(ShrdTracker::aliveCount, 0);
}