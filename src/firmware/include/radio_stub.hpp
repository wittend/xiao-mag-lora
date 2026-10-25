#pragma once
#include <cstdint>
#include <cstring>
#include <deque>
#include <vector>

#include "radio.hpp"

namespace xlora::radio {

// In-memory Radio used by the host unit tests (and as the placeholder until the
// real SX1262 driver is integrated). Records every TX and serves back frames
// injected via inject_rx(), so encode -> send -> poll -> decode can be tested
// end-to-end without hardware.
class StubRadio final : public Radio {
public:
    bool begin() override { return true; }
    void set_spreading_factor(int sf) override { sf_ = sf; }
    void set_frequency_hz(std::uint32_t f) override { freq_ = f; }

    bool send(const std::uint8_t* data, std::size_t len) override {
        txs_.emplace_back(data, data + len);
        return true;
    }

    bool poll(std::uint8_t* out, std::size_t cap, std::size_t& out_len) override {
        if (rx_.empty()) return false;
        std::size_t n = rx_.front().size();
        if (n > cap) n = cap;
        std::memcpy(out, rx_.front().data(), n);
        out_len = n;
        rx_.pop_front();
        return true;
    }

    // --- test / inspection hooks ---
    void inject_rx(std::vector<std::uint8_t> b) { rx_.push_back(std::move(b)); }
    std::size_t tx_count() const { return txs_.size(); }
    const std::vector<std::uint8_t>& last_tx() const { return txs_.back(); }
    int sf() const { return sf_; }
    std::uint32_t freq_hz() const { return freq_; }

private:
    std::deque<std::vector<std::uint8_t>> txs_;
    std::deque<std::vector<std::uint8_t>> rx_;
    int sf_ = 9;
    std::uint32_t freq_ = 915000000;
};

}  // namespace xlora::radio
