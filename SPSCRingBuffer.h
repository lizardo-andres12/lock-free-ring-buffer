#include <atomic>
#include <bitset>
#include <cstddef>
#include <memory>
#include <new>
#include <stdexcept>


template<typename T, size_t N>
class SPSCRingBuffer {
    size_t capacity_;
    std::unique_ptr<T[]> queue_;

    alignas(64) std::atomic<size_t> readIdx_ {0};
    alignas(64) std::atomic<size_t> writeIdx_ {0};

public:
    SPSCRingBuffer() :
	capacity_(N),
	queue_(std::make_unique<T[]>(N))
    {
	if (N < 2 || std::bitset<sizeof(size_t) * 8>(N).count() > 1) {
	    throw std::logic_error("Cannot instantiate SPSC Queue with size that is not power of 2");
	}
    }

    SPSCRingBuffer(const SPSCRingBuffer<T, N>& spsc) = delete;
    SPSCRingBuffer(SPSCRingBuffer<T, N>&& spsc) = delete;
    void operator=(const SPSCRingBuffer<T, N>& spsc) = delete;
    void operator=(SPSCRingBuffer<T, N>&& spsc) = delete;
    ~SPSCRingBuffer() = default;
    
    // Producer side
    bool try_push(const T& item);
    
    // Consumer side
    bool try_pop(T& item);
    
    // Utility
    size_t size() const {
	const auto readIdx = readIdx_.load(std::memory_order_relaxed);
	const auto writeIdx = writeIdx_.load(std::memory_order_relaxed);

	if (writeIdx > readIdx) {
	    return writeIdx - readIdx;
	} else {
	    return capacity_ - (readIdx - writeIdx);
	}
    }


    bool empty() const {
	const auto readIdx = readIdx_.load(std::memory_order_relaxed);
	const auto writeIdx = writeIdx_.load(std::memory_order_relaxed);
	return readIdx == writeIdx;
    }

    bool full() const {
	const auto readIdx = readIdx_.load(std::memory_order_relaxed);
	const auto writeIdx = writeIdx_.load(std::memory_order_relaxed);
	const auto nextWriteIdx = (writeIdx + 1) & (capacity_ - 1);
	return readIdx == nextWriteIdx;
    }
};

template <typename T, size_t N>
bool SPSCRingBuffer<T, N>::try_push(const T& item) {
    const auto writeIdx = writeIdx_.load(std::memory_order_relaxed);
    const auto readIdx = readIdx_.load(std::memory_order_acquire);

    const auto nextWriteIdx = (writeIdx + 1) & (capacity_ - 1);
    if (nextWriteIdx == readIdx) {
	return false;
    }

    queue_[writeIdx] = std::move(item);
    writeIdx_.store(nextWriteIdx, std::memory_order_release);
    return true;
}

template <typename T, size_t N>
bool SPSCRingBuffer<T, N>::try_pop(T& item) {
    const auto readIdx = readIdx_.load(std::memory_order_relaxed);
    const auto writeIdx = writeIdx_.load(std::memory_order_acquire);

    if (readIdx == writeIdx) {
	return false;
    }

    item = std::move(queue_[readIdx]);

    const auto nextReadIdx = (readIdx + 1) & (capacity_ - 1);
    readIdx_.store(nextReadIdx, std::memory_order_release);
    return true;
}

