#ifndef symbols_h
#define symbols_h

#include <cstdint>
#include <variant>
#include <vector>

struct Match {
    int length;     // copy - past  [abc <--l--> abc]
    int distance;   // go back      [x <--d-- x]
};

using Token = std::variant<unsigned char, Match>;

struct LiteralEvent { uint16_t symbol; };
struct MatchEvent   { uint16_t lenSym, lenExtraBits, lenExtraVal, 
                                distSym, distExtraBits, distExtraVal; };
struct EndEvent     { uint16_t symbol = 256; };

using Event = std::variant<LiteralEvent, MatchEvent, EndEvent>;

class Symbols
{
private:
    static constexpr int length_base[29] = {
        3, 4, 5, 6, 7, 8, 9, 10,
        11, 13, 15, 17,
        19, 23, 27, 31,
        35, 43, 51, 59,
        67, 83, 99, 115,
        131, 163, 195, 227,
        258
    };

    static constexpr int length_extra[29] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1,
        2, 2, 2, 2,
        3, 3, 3, 3,
        4, 4, 4, 4,
        5, 5, 5, 5,
        0
    };

    static constexpr int distance_base[30] = {
        1, 2, 3, 4,
        5, 7,
        9, 13,
        17, 25,
        33, 49,
        65, 97,
        129, 193,
        257, 385,
        513, 769,
        1025, 1537,
        2049, 3073,
        4097, 6145,
        8193, 12289,
        16385, 24577
    };

    static constexpr int distance_extra[30] = {
        0, 0, 0, 0,
        1, 1,
        2, 2,
        3, 3,
        4, 4,
        5, 5,
        6, 6,
        7, 7,
        8, 8,
        9, 9,
        10, 10,
        11, 11,
        12, 12,
        13, 13
    };

    static void encodeLength(int length, uint16_t &symbol, uint16_t &extraBits, uint16_t &extraVal);

    static void encodeDistance(int distance, uint16_t &symbol, uint16_t &extraBits, uint16_t &extraVal);

    static int decodeLength(uint16_t symbol, uint16_t extraValue);

    static int decodeDistance(uint16_t symbol, uint16_t extraValue);

public:
    static std::vector<Event> encode(const std::vector<Token> &tokens);

    static std::vector<Token> decode(const std::vector<Event> &events);
};

#endif