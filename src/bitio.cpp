#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "bitio.hpp"

class BitWriter {
private:
    std::vector<uint8_t> buf_;
    uint8_t cur_ = 0;
    uint32_t nbits_ = 0;
public:
    void write_bit(uint32_t bit) {
        cur_ = static_cast<uint8_t>((cur_ << 1) | (bit & 1u));
        if (++nbits_ == 8) {
            buf_.push_back(cur_);
            cur_ = 0;
            nbits_ = 0;
        }
    }

    void write_bits(uint32_t value, uint32_t count) {
        for (uint32_t i = count; i > 0; --i)
            write_bit((value >> (i - 1)) & 1u);
    }

    void write_bitstring(const std::string& s) {
        for (char c : s) write_bit(c == '1' ? 1u : 0u);
    }

    void flush() {
        while (nbits_ != 0) write_bit(0);
    }

    const std::vector<uint8_t>& bytes() const { return buf_; }

};

class BitReader {
private:
    const std::vector<uint8_t>& data_;
    size_t pos_ = 0;
public:
    explicit BitReader(const std::vector<uint8_t>& data) : data_(data) {}

    bool eof() const { return pos_ >= data_.size() * 8; }

    uint32_t read_bit() {
        if (eof()) throw std::runtime_error("BitReader: unexpected end of data");
        uint32_t byte = data_[pos_ >> 3];
        uint32_t bit = (byte >> (7 - (pos_ & 7))) & 1u;
        ++pos_;
        return bit;
    }

    uint32_t read_bits(uint32_t count) {
        uint32_t v = 0;
        for (uint32_t i = 0; i < count; ++i)
            v = (v << 1) | read_bit();
        return v;
    }
};