#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "bitio.hpp"


void BitWriter::write_bit(uint32_t bit) {
    cur_ = static_cast<uint8_t>((cur_ << 1) | (bit & 1u));
    if (++nbits_ == 8) {
        buf_.push_back(cur_);
        cur_ = 0;
        nbits_ = 0;
    }
}

void BitWriter::write_bits(uint32_t value, uint32_t count) {
    for (uint32_t i = count; i > 0; --i)
        write_bit((value >> (i - 1)) & 1u);
}

void BitWriter::write_bitstring(const std::string& s) {
    for (char c : s) write_bit(c == '1' ? 1u : 0u);
}

void BitWriter::flush() {
    while (nbits_ != 0) write_bit(0);
}

const std::vector<uint8_t>& BitWriter::bytes() const { return buf_; }

//------------------------------------------------------------------------------------------

bool BitReader::eof() const { return pos_ >= data_.size() * 8; }

uint32_t BitReader::read_bit() {
    if (eof()) throw std::runtime_error("BitReader: unexpected end of data");
    uint32_t byte = data_[pos_ >> 3];
    uint32_t bit = (byte >> (7 - (pos_ & 7))) & 1u;
    ++pos_;
    return bit;
}

uint32_t BitReader::read_bits(uint32_t count) {
    uint32_t v = 0;
    for (uint32_t i = 0; i < count; ++i)
        v = (v << 1) | read_bit();
    return v;
}
