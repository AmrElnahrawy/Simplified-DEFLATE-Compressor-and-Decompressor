#include <cstdint>
#include <variant>
#include <vector>
#include <stdexcept>
#include "symbols.hpp"

struct Match {
    int length;     // copy - past  [abc <--l--> abc]
    int distance;   // go back      [x <--d-- x]
};

using Token = std::variant<unsigned char, Match>;

struct LiteralEvent { uint16_t symbol; };
struct MatchEvent   { uint16_t lenSym, lenExtraBits, lenExtraVal, 
                                distSym, distExtraBits, distExtraVal; };
struct EndEvent      { uint16_t symbol = 256; };

using Event = std::variant<LiteralEvent, MatchEvent, EndEvent>;

class Symbols
{
private:
    static constexpr uint16_t length_base[29] = {
        3, 4, 5, 6, 7, 8, 9, 10,
        11, 13, 15, 17,
        19, 23, 27, 31,
        35, 43, 51, 59,
        67, 83, 99, 115,
        131, 163, 195, 227,
        258
    };

    static constexpr uint16_t length_extra[29] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1,
        2, 2, 2, 2,
        3, 3, 3, 3,
        4, 4, 4, 4,
        5, 5, 5, 5,
        0
    };

    static constexpr uint16_t distance_base[30] = {
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

    static constexpr uint16_t distance_extra[30] = {
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

    static void encodeLength(int length, uint16_t &symbol, uint16_t &extraBits, uint16_t &extraVal) {
        int i = 0;
        while (i < 28 && length >= length_base[i + 1]) 
            i++;
        symbol   = 257 + i;
        extraBits = length_extra[i];
        extraVal  = length - length_base[i];
    }

    static void encodeDistance(int distance, uint16_t &symbol, uint16_t &extraBits, uint16_t &extraVal) {
        int i = 0;
        while (i < 29 && distance >= distance_base[i + 1])
             i++;
        symbol    = i;
        extraBits = distance_extra[i];
        extraVal  = distance - distance_base[i];
    }

    static int decodeLength(uint16_t symbol, uint16_t extraValue) {
        int i = symbol - 257;
        return length_base[i] + extraValue;
    }

    static int decodeDistance(uint16_t symbol, uint16_t extraValue) {
        return distance_base[symbol] + extraValue;
    }

public:
    static std::vector<Event> encode(const std::vector<Token> &tokens) {
        std::vector<Event> events;
        events.reserve(tokens.size() + 1);
        for (const auto& tok : tokens) {
            if (tok.index() == 0) {
                events.push_back(LiteralEvent{ std::get<0>(tok) });
            } else {
                const auto& m = std::get<1>(tok);
                MatchEvent e{};
                encodeLength(m.length, e.lenSym, e.lenExtraBits, e.lenExtraVal);
                encodeDistance(m.distance, e.distSym, e.distExtraBits, e.distExtraVal);
                events.push_back(e);
            }
        }
        events.push_back(EndEvent{});
        return events;
    }

    static std::vector<Token> decode(const std::vector<Event> &events) {
        std::vector<Token> tokens;
        tokens.reserve(events.size());

        for (const auto& ev : events) {
            switch (ev.index()) {
                case 0: { 
                    const auto& lit = std::get<LiteralEvent>(ev);
                    tokens.push_back(static_cast<unsigned char>(lit.symbol));
                    break;
                }
                case 1: { 
                    const auto& m = std::get<MatchEvent>(ev);
                    int length   = decodeLength(m.lenSym, m.lenExtraVal);
                    int distance = decodeDistance(m.distSym, m.distExtraVal);
                    tokens.push_back(Match{ length, distance });
                    break;
                }
                case 2:
                    return tokens;
                default:
                    throw std::logic_error("unreachable variant index");
            }
        }

        throw std::runtime_error("Event stream missing EndEvent(256) terminator");
    }
};