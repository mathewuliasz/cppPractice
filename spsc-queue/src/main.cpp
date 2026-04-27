#include <spsc/ring_buffer.hpp>
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include <cstdint>

void test_name_describing_what_you_test()
{
    // setup
    // action
    // assert
    RingBuffer<int, 4> rb;
    assert(rb.empty());
    assert(rb.size() == 0);
    std::cout << "PASSED: test_name_describing_what_you_test\n";
}

int main()
{
    RingBuffer<uint64_t, 65536> rb;
    constexpr uint64_t count = 10000000;

    auto start = std::chrono::steady_clock::now();

    std::thread producer([&]
                         {
        for (uint64_t i = 0; i < count; i++){
            while(!rb.try_push(i));
        } });

    std::thread consumer([&]
                         {
        uint64_t val;
        uint64_t received = 0;
        while(received < count) {
            if (rb.try_pop(val))
                received++;
        } });

    producer.join();
    consumer.join();

    auto end = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    double ops_per_sec = static_cast<double>(count) / (ms / 1000.0);

    std::cout << count << " items in " << ms << " ms\n";
    std::cout << static_cast<uint64_t>(ops_per_sec) << " ops/sec\n";

    return 0;
}
