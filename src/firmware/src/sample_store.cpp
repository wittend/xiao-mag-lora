#include "sample_store.hpp"

namespace xlora {

void SampleStore::push(const frame::Sample& s) {
    if (size_ == cap_) {
        // Full: overwrite the oldest (at head_), which drops it.
        buf_[head_] = s;
        ++dropped_;
        head_ = (head_ + 1) % cap_;
    } else {
        buf_[(head_ + size_) % cap_] = s;
        ++size_;
    }
}

bool SampleStore::pop(frame::Sample& out) {
    if (size_ == 0) return false;
    out = buf_[head_];
    head_ = (head_ + 1) % cap_;
    --size_;
    return true;
}

}  // namespace xlora
