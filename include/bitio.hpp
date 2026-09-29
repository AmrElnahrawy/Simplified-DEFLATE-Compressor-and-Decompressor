#ifndef bitio_h
#define bitio_h

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class BitWriter {
private:
    std::vector<uint8_t> buf_;
    uint8_t cur_ = 0;
    uint32_t nbits_ = 0;
public:
    void write_bit(uint32_t bit);

    void write_bits(uint32_t value, uint32_t count);

    void write_bitstring(const std::string& s);

    void flush();

    const std::vector<uint8_t>& bytes() const;

};

class BitReader {
private:
    const std::vector<uint8_t>& data_;
    size_t pos_ = 0;
public:
    explicit BitReader(const std::vector<uint8_t>& data) : data_(data) {}

    bool eof() const;

    uint32_t read_bit();

    uint32_t read_bits(uint32_t count);
};

#endif