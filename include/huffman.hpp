#ifndef huffman_h
#define huffman_h
#include <cstdint>
#include <variant>
#include <vector>

#include "huffman.hpp"

struct LiteralEvent { uint16_t symbol; };
struct MatchEvent   { uint16_t lenSym, lenExtraBits, lenExtraVal, 
                                distSym, distExtraBits, distExtraVal; };
struct EndEvent     { uint16_t symbol = 256; };

using Event = std::variant<LiteralEvent, MatchEvent, EndEvent>;

struct huffmanNode
{
    uint16_t symbol;
    uint32_t freq;
    uint16_t min_symbol;
    huffmanNode* left = nullptr;
    huffmanNode* right = nullptr;

    huffmanNode(uint16_t symbol, uint32_t freq, uint16_t min_symbol) : symbol(symbol), freq(freq), min_symbol(min_symbol), left(nullptr), right(nullptr) {}

    ~huffmanNode() {
        delete left;
        delete right;
    }

    bool operator<(const huffmanNode& other) const {
        if (freq != other.freq) {
            return freq < other.freq; 
        }
        return min_symbol < other.min_symbol; 
    }
};


class huffman
{
private:
    static constexpr int LLS = 286;
    static constexpr int DS = 30;

    std::vector<uint32_t> literal_length_freqs = std::vector<uint32_t>(LLS, 0);
    std::vector<uint32_t> distance_freqs       = std::vector<uint32_t>(DS, 0);

    std::vector<uint32_t> ll_code_lengths = std::vector<uint32_t>(LLS, 0);
    std::vector<uint32_t> d_code_lengths  = std::vector<uint32_t>(DS, 0);

    std::vector<uint32_t> ll_codes = std::vector<uint32_t>(LLS, 0);
    std::vector<uint32_t> d_codes  = std::vector<uint32_t>(DS, 0);

    void calculate_frequencies(const std::vector<Event> &events);

    huffmanNode* huffman_tree(std::vector<uint32_t> freqs);

    void count_lengths(huffmanNode* root, uint32_t depth, std::vector<uint32_t>& x_code_length);

    std::vector<uint32_t> canonical_codes(const std::vector<uint32_t>& lengths);
    
public:
    void encode(const std::vector<Event>& events);

    const std::vector<uint32_t>& get_ll_codes() const { return ll_codes; }
    const std::vector<uint32_t>& get_d_codes()  const { return d_codes; }

    const std::vector<uint32_t>& get_ll_code_lengths() const { return ll_code_lengths; }
    const std::vector<uint32_t>& get_d_code_lengths()  const { return d_code_lengths; }

};
#endif