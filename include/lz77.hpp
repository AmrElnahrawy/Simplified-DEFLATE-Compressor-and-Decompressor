#ifndef lz77_h
#define lz77_h
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <variant>

struct Match
{
    int length;
    int distance;
};

using Token = std::variant<unsigned char, Match>;

struct RingBuffer {
    std::vector<uint32_t> indices;
    int head = 0;

    void push(int pos) {
        if (indices.size() < 64) {
            indices.push_back(pos);
        } else {
            indices[head] = pos;
            head = (head + 1) % 64;
        }
    }
};


class LZ77
{
private:
    static constexpr int WINDOW_SIZE = 32768;
    static constexpr int MIN_MATCH = 3;
    static constexpr int MAX_MATCH = 258;
    static constexpr int MAX_CANDIDATES = 64;
    
public:
    static std::vector<Token> encode(const std::vector<unsigned char> &data);

    static std::vector<unsigned char> decode(const std::vector<Token> &tokens);
};

#endif