#pragma once
#include <cstdint>

namespace xlora::link {

// Link-health counters surfaced to the client / SPA.
struct Stats {
    std::uint32_t tx = 0;
    std::uint32_t rx = 0;
    std::uint32_t crc_err = 0;
    std::uint32_t lost = 0;  // derived from sequence gaps
};

// Monotonic sequence allocator wrapping 0..65535, one per direction.
class TxSeq {
public:
    explicit TxSeq(std::uint16_t start = 0) : next_(start) {}
    std::uint16_t alloc() {
        std::uint16_t v = next_;
        ++next_;  // wraps naturally
        return v;
    }
    void reset(std::uint16_t s) { next_ = s; }

private:
    std::uint16_t next_;
};

// In-order sequence-gap (loss) estimator for a wrapping uint16 stream.
// Assumes no retransmit/reorder (best-effort P2P, no line-rate retry). Sequences
// that are "behind" the last seen (delta > 32768) are treated as duplicates.
class RxGaps {
public:
    void observe(std::uint16_t seq) {
        if (!have_) { last_ = seq; have_ = true; return; }
        std::uint32_t delta =
            (static_cast<std::uint32_t>(seq) - static_cast<std::uint32_t>(last_)) & 0xFFFFu;
        if (delta > 32768u) return;      // backward / duplicate: ignore
        if (delta == 0u) return;         // exact duplicate: ignore
        if (delta >= 2u) lost_ += static_cast<std::uint32_t>(delta - 1u);
        last_ = seq;
    }
    std::uint32_t lost() const { return lost_; }
    void reset() { have_ = false; lost_ = 0; }

private:
    bool have_ = false;
    std::uint16_t last_ = 0;
    std::uint32_t lost_ = 0;
};

}  // namespace xlora::link
