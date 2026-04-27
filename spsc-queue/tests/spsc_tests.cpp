#include <spsc/ring_buffer.hpp>
#include <gtest/gtest.h>
#include <thread>
#include <string>

TEST(RingBuffer, EmptyOnInit)
{
    RingBuffer<int, 4> rb;
    EXPECT_TRUE(rb.empty());
    EXPECT_EQ(rb.capacity(), 4);
    EXPECT_EQ(rb.size(), 0);
    EXPECT_FALSE(rb.full());
}

TEST(RingBuffer, FullQueue)
{
    RingBuffer<int, 4> rb;
    EXPECT_TRUE(rb.try_push(1));
    EXPECT_TRUE(rb.try_push(1));
    EXPECT_TRUE(rb.try_push(1));
    EXPECT_TRUE(rb.try_push(1));
    EXPECT_FALSE(rb.empty());
    EXPECT_TRUE(rb.full());
    EXPECT_FALSE(rb.try_push(1));
}

TEST(RingBuffer, PushPopOperations)
{
    RingBuffer<int, 2> rb;
    EXPECT_TRUE(rb.try_push(10));
    EXPECT_TRUE(rb.try_push(20));
    int val;
    EXPECT_TRUE(rb.try_pop(val));
    EXPECT_EQ(val, 10);
    EXPECT_TRUE(rb.try_pop(val));
    EXPECT_EQ(val, 20);
}

TEST(RingBuffer, FullReturnsFalse)
{
    RingBuffer<int, 2> rb;
    EXPECT_TRUE(rb.try_push(1));
    EXPECT_TRUE(rb.try_push(2));
    EXPECT_FALSE(rb.try_push(3));
}

TEST(RingBuffer, EmptyReturnsFalse)
{
    RingBuffer<int, 2> rb;
    int val;
    EXPECT_FALSE(rb.try_pop(val));
}

TEST(RingBuffer, Wraparound)
{
    RingBuffer<int, 4> rb;
    for (int round = 0; round < 10; round++)
    {
        for (int i = 0; i < 4; i++)
            EXPECT_TRUE(rb.try_push(round * 100 + i));
        EXPECT_TRUE(rb.full());
        for (int i = 0; i < 4; i++)
        {
            int val;
            EXPECT_TRUE(rb.try_pop(val));
            EXPECT_EQ(val, round * 100 + i);
        }
        EXPECT_TRUE(rb.empty());
    }
}

TEST(RingBuffer, MoveSemantics)
{
    RingBuffer<std::string, 4> rb;
    std::string s = "hello";
    EXPECT_TRUE(rb.try_push(std::move(s)));
    EXPECT_TRUE(s.empty());

    std::string out;
    EXPECT_TRUE(rb.try_pop(out));
    EXPECT_EQ(out, "hello");
}

TEST(RingBuffer, DestructorCleansUp)
{
    {
        RingBuffer<std::string, 4> rb;
        rb.try_push("one");
        rb.try_push("two");
        rb.try_push("three");
        rb.try_push("four");
    }
    SUCCEED();
}

TEST(RingBuffer, ConcurrentCorrectness)
{
    RingBuffer<uint64_t, 1024> rb;
    constexpr uint64_t count = 1000000;
    std::thread producer([&]
                         {
        for (uint64_t i = 0; i < count; i++){
            while(!rb.try_push(i));
        } });

    std::thread consumer([&]
                         {

        uint64_t expected = 0;
        while( expected < count){
            uint64_t val;
            if(rb.try_pop(val)) {
                EXPECT_EQ(val, expected);
                expected++;
            }
        } });

    producer.join();
    consumer.join();
    EXPECT_TRUE(rb.empty());
}