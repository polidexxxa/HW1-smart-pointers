#include <gtest/gtest.h>
#include <utility>
#include "UnqPtr.hpp"

struct UnqTracker {
    static int aliveCount;
    int value = 0;

    UnqTracker() : value(0) { ++aliveCount; }
    explicit UnqTracker(int v) : value(v) { ++aliveCount; }
    UnqTracker(const UnqTracker& other) : value(other.value) { ++aliveCount; }
    UnqTracker(UnqTracker&& other) noexcept : value(other.value) { ++aliveCount; }
    ~UnqTracker() { --aliveCount; }
};
int UnqTracker::aliveCount = 0;

struct UnqBase {
    int id = 10;
    virtual ~UnqBase() = default;
    virtual int Calc() const { return id; }
};

struct UnqDerived : public UnqBase {
    int extra = 20;
    int Calc() const override { return id + extra; }
};

class UnqPtrFixture : public ::testing::Test {
protected:
    void SetUp() override {
        UnqTracker::aliveCount = 0;
    }
    void TearDown() override {
        EXPECT_EQ(UnqTracker::aliveCount, 0);
    }
};

TEST_F(UnqPtrFixture, NullptrAndDefaultState) {
    UnqPtr<int> p1;
    EXPECT_FALSE(p1);
    EXPECT_EQ(p1.Get(), nullptr);

    UnqPtr<int> p2(nullptr);
    EXPECT_FALSE(p2);
    EXPECT_EQ(p2.Get(), nullptr);
}

TEST_F(UnqPtrFixture, LifetimeManagement) {
    {
        auto p = MakeUnq<UnqTracker>(123);
        EXPECT_EQ(UnqTracker::aliveCount, 1);
        EXPECT_EQ(p->value, 123);
        EXPECT_EQ((*p).value, 123);
    }
    EXPECT_EQ(UnqTracker::aliveCount, 0);
}

TEST_F(UnqPtrFixture, MoveSemantics) {
    auto p1 = MakeUnq<UnqTracker>(50);
    EXPECT_EQ(UnqTracker::aliveCount, 1);

    UnqPtr<UnqTracker> p2 = std::move(p1);
    EXPECT_FALSE(p1);
    EXPECT_EQ(p1.Get(), nullptr);
    EXPECT_TRUE(p2);
    EXPECT_EQ(p2->value, 50);
    EXPECT_EQ(UnqTracker::aliveCount, 1);

    UnqPtr<UnqTracker> p3;
    p3 = std::move(p2);
    EXPECT_FALSE(p2);
    EXPECT_TRUE(p3);
    EXPECT_EQ(p3->value, 50);
}

TEST_F(UnqPtrFixture, ResetAndRelease) {
    auto p = MakeUnq<UnqTracker>(77);
    UnqTracker* raw = p.Release();
    EXPECT_FALSE(p);
    EXPECT_EQ(UnqTracker::aliveCount, 1);

    delete raw;
    EXPECT_EQ(UnqTracker::aliveCount, 0);

    p.Reset(new UnqTracker(88));
    EXPECT_EQ(UnqTracker::aliveCount, 1);
    EXPECT_EQ(p->value, 88);

    p.Reset();
    EXPECT_EQ(UnqTracker::aliveCount, 0);
    EXPECT_FALSE(p);
}

TEST_F(UnqPtrFixture, UpcastingPolymorphism) {
    UnqPtr<UnqDerived> d = MakeUnq<UnqDerived>();
    EXPECT_EQ(d->Calc(), 30);

    UnqPtr<UnqBase> b = std::move(d);
    EXPECT_FALSE(d);
    EXPECT_TRUE(b);
    EXPECT_EQ(b->Calc(), 30);
}

TEST_F(UnqPtrFixture, ArraySpecialization) {
    const size_t kSize = 5;
    {
        auto arr = MakeUnq<UnqTracker[]>(kSize);
        EXPECT_EQ(UnqTracker::aliveCount, 5);

        for (size_t i = 0; i < kSize; ++i) {
            arr[i].value = static_cast<int>(i + 1);
        }
        for (size_t i = 0; i < kSize; ++i) {
            EXPECT_EQ(arr[i].value, static_cast<int>(i + 1));
        }
    }
    EXPECT_EQ(UnqTracker::aliveCount, 0);
}

TEST_F(UnqPtrFixture, ArrayMoveSemantics) {
    const size_t kSize = 6;
    auto arr1 = MakeUnq<UnqTracker[]>(kSize);
    EXPECT_EQ(UnqTracker::aliveCount, 6);

    for (size_t i = 0; i < kSize; ++i) {
        arr1[i].value = static_cast<int>(i * 5);
    }

    UnqPtr<UnqTracker[]> arr2 = std::move(arr1);
    EXPECT_FALSE(arr1);
    EXPECT_EQ(arr1.Get(), nullptr);
    EXPECT_TRUE(arr2);
    EXPECT_EQ(UnqTracker::aliveCount, 6);

    for (size_t i = 0; i < kSize; ++i) {
        EXPECT_EQ(arr2[i].value, static_cast<int>(i * 5));
    }

    UnqPtr<UnqTracker[]> arr3;
    arr3 = std::move(arr2);
    EXPECT_FALSE(arr2);
    EXPECT_TRUE(arr3);
    EXPECT_EQ(arr3[3].value, 15);
}

TEST_F(UnqPtrFixture, ArrayResetReplacesBuffer) {
    auto arr = MakeUnq<UnqTracker[]>(4);
    EXPECT_EQ(UnqTracker::aliveCount, 4);

    arr.Reset(new UnqTracker[8]());
    EXPECT_EQ(UnqTracker::aliveCount, 8);

    arr.Reset();
    EXPECT_EQ(UnqTracker::aliveCount, 0);
    EXPECT_FALSE(arr);
}

TEST_F(UnqPtrFixture, ConstArrayAccess) {
    auto arr = MakeUnq<int[]>(3);
    arr[0] = 10;
    arr[1] = 20;
    arr[2] = 30;

    const auto& constRef = arr;
    EXPECT_EQ(constRef[0], 10);
    EXPECT_EQ(constRef[1], 20);
    EXPECT_EQ(constRef[2], 30);
    EXPECT_NE(constRef.Get(), nullptr);
}