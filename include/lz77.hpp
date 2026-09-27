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


class lz77
{
private:
    const std::vector<unsigned char> &data;
    const int WINDOW_SIZE = 32768;
    const int MIN_MATCH = 3;
    const int MAX_MATCH = 258;
    const int MAX_CANDIDATES = 64;
    
public:
    lz77(const std::vector<unsigned char>& inputdata) : data(inputdata) {}
    ~lz77() = default;

    std::vector<Token> encode();

    std::vector<unsigned char> decode(const std::vector<Token> &tokens_list);
};

#endif