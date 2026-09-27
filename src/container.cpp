#include <cstdint>
#include <vector>


#include "container.hpp"
namespace sdfl {
    std::vector<uint8_t> compress(const std::vector<uint8_t>& input);
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& compressed);
}