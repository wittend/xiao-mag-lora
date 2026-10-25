#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

#include "lora_frame.hpp"

namespace xlora {

// Fixed-capacity ring buffer of accumulated samples awaiting a radio flush.
// On overflow the oldest sample is dropped and counted (loss-tolerant backlog:
// we always keep the most recent N).
class SampleStore {
public:
    explicit SampleStore(std::size_t capacity)
        : buf_(capacity ? capacity : 1), cap_(capacity ? capacity : 1) {}

    void push(const frame::Sample& s);
    bool pop(frame::Sample& out);  // false if empty

    std::size_t  size() const { return size_; }
    std::size_t  capacity() const { return cap_; }
    std::uint64_t dropped() const { return dropped_; }

private:
    std::vector<frame::Sample> buf_;
    std::size_t cap_;
    std::size_t head_ = 0;    // index of oldest element
    std::size_t size_ = 0;
    std::uint64_t dropped_ = 0;
};

}  // namespace xlora
