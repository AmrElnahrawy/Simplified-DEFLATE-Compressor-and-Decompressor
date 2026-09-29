#ifndef types_hpp
#define types_hpp

#include <cstdint>
#include <variant>

// Stage 1: LZ77 tokens
struct Match {
    int length;     // copy - past  [abc <--l--> abc]
    int distance;   // go back      [x <--d-- x]
};

using Token = std::variant<unsigned char, Match>;

// Stage 2: DEFLATE events
struct LiteralEvent { uint16_t symbol; };

struct MatchEvent {
    uint16_t lenSym, lenExtraBits, lenExtraVal,
             distSym, distExtraBits, distExtraVal;
};

struct EndEvent { uint16_t symbol = 256; };

using Event = std::variant<LiteralEvent, MatchEvent, EndEvent>;

#endif