SPSC Queue

How to compile:
cmake -B build -G Ninja
cmake --build build

How to run benchmarks:
./build/spsc_demo (recompile after each code change)

Benchmark comparisons:

1. w/o optimizaitions: avg. 10000000 items in 727 ms , 13,784,209 ops/sec

2. w/ cache line padding for each cursor (push/pop): 10000000 items in 565 ms , 17,747,380 ops/sec ~30% speedup
   -uncomment alignas(64) cursors, and comment off currently used cursors.

3. w/ power-of-two masking: 100000000 items in 610 ms, 16,800,000 ops/sec ~no visible gain but used for next optimizations
   -replace % N everywhere w/ & (N - 1)

4. w/ locally cached cursors & memory order relaxation (atomic vars): 10000000 items in 449 ms , 22,072,091 ops/sec ~30% speedup
