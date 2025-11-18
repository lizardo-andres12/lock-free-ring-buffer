## **Task 5: Lock-Free Ring Buffer for Market Data**
**Difficulty: Advanced**

Market data comes from multiple threads (different exchanges, different feeds). You need a **lock-free** data structure to pass messages between threads without blocking.
A traditional queue with mutexes is too slow - every lock/unlock costs ~50-100ns.

I've implemented a **Single Producer, Single Consumer (SPSC) lock-free ring buffer**.

### Background:

**Ring Buffer (Circular Buffer):**
```
[0][1][2][3][4][5][6][7]
 ^                   ^
head                tail
```

- **Producer** writes at `tail`, increments tail
- **Consumer** reads at `head`, increments head  
- When `tail` catches up to `head`, buffer is full
- When `head` catches up to `tail`, buffer is empty

**Lock-Free:** No mutexes. Uses atomic operations and memory ordering.

---

### Requirements:

Implement a ring buffer with this interface:

```cpp
template<typename T, size_t N>
class SPSCRingBuffer {
public:
    SPSCRingBuffer();
    
    // Producer side
    bool try_push(const T& item);
    
    // Consumer side
    bool try_pop(T& item);
    
    // Utility
    size_t size() const;
    bool empty() const;
    bool full() const;
};
```

Then write a test program that:
1. **Producer thread:** Generates 10,000,000 integers (0, 1, 2, ..., 9999999) and pushes them
2. **Consumer thread:** Pops integers and verifies they're in order
3. **Measures:** Total throughput (items/second)

**Constraints:**
- Buffer size: 1024 elements
- Must be **truly lock-free** (no mutexes, no spinlocks)
- Must handle wraparound correctly
- No data corruption, no lost messages
- Keep it under 300 lines

#### **False Sharing Prevention**

If `head_` and `tail_` are on the same cache line, the producer and consumer will fight over it:

```cpp
alignas(std::hardware_destructive_interference_size) std::atomic<size_t> head_;  // Own cache line
alignas(std::hardware_destructive_interference_size) std::atomic<size_t> tail_;  // Own cache line
```

`std::hardware_destructive_interference_size` is a constant that represents the minimum offset between two memory addresses to ensure they do not reside on the same cache line, thus preventing false sharing.

---

### Performance Targets:

On a modern CPU (Ryzen/Intel Core):
- **Bad implementation (with mutex):** ~5-10M items/sec
- **Good lock-free:** ~50-100M items/sec
- **Excellent lock-free:** ~200-500M items/sec
