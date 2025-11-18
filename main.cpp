#include <chrono>
#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <thread>

#include "SPSCRingBuffer.h"

const size_t NUM_ITEMS = 10'000'000;

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout << std::fixed << std::setprecision(2);

    SPSCRingBuffer<int, 1024> buf;

    auto start = std::chrono::high_resolution_clock::now();

    std::thread producer([&buf]() {
	for (size_t i = 0; i < NUM_ITEMS; ++i) {
	    while (!buf.try_push(i)) {
		std::this_thread::yield();
	    }
	}
    });

    std::thread consumer([&buf]() {
	int expected = 0;
	int item;

	while (expected < NUM_ITEMS) {
	    if (buf.try_pop(item)) {
		if (item != expected) {
		    std::cerr << "Error: got " << item << " but expected " << expected << '\n';
		    std::terminate();
		}
		expected++;
	    }
	}
    });

    producer.join();
    consumer.join();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    double throughput = NUM_ITEMS / (duration.count() / 1000.0);

    std::cout << "Processed " << NUM_ITEMS << " items in " << duration.count() << "ms\n";
    std::cout << "Throughput: " << throughput << " items per second\n";
    return 0;
}
