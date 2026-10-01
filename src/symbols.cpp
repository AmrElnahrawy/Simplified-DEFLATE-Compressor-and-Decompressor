#include <cstdint>
#include <variant>
#include <vector>
#include <stdexcept>
#include "symbols.hpp"

void Symbols::encodeLength(int length, uint16_t &symbol, uint16_t &extraBits, uint16_t &extraVal)
{
    int i = 0;
    while (i < 28 && length >= length_base[i + 1])
        i++;
    symbol = 257 + i;
    extraBits = length_extra[i];
    extraVal = length - length_base[i];
}

void Symbols::encodeDistance(int distance, uint16_t &symbol, uint16_t &extraBits, uint16_t &extraVal)
{
    int i = 0;
    while (i < 29 && distance >= distance_base[i + 1])
        i++;
    symbol = i;
    extraBits = distance_extra[i];
    extraVal = distance - distance_base[i];
}

int Symbols::decodeLength(uint16_t symbol, uint16_t extraValue)
{
    int i = symbol - 257;
    return length_base[i] + extraValue;
}

int Symbols::decodeDistance(uint16_t symbol, uint16_t extraValue)
{
    return distance_base[symbol] + extraValue;
}

std::vector<Event> Symbols::encode(const std::vector<Token> &tokens)
{
    std::vector<Event> events;
    events.reserve(tokens.size() + 1);
    for (const auto &tok : tokens)
    {
        if (tok.index() == 0)
        {
            events.push_back(LiteralEvent{std::get<0>(tok)});
        }
        else
        {
            const auto &m = std::get<1>(tok);
            MatchEvent e{};
            encodeLength(m.length, e.lenSym, e.lenExtraBits, e.lenExtraVal);
            encodeDistance(m.distance, e.distSym, e.distExtraBits, e.distExtraVal);
            events.push_back(e);
        }
    }
    events.push_back(EndEvent{});
    return events;
}

std::vector<Token> Symbols::decode(const std::vector<Event> &events)
{
    std::vector<Token> tokens;
    tokens.reserve(events.size());

    for (const auto &ev : events)
    {
        switch (ev.index())
        {
        case 0:
        {
            const auto &lit = std::get<LiteralEvent>(ev);
            tokens.push_back(static_cast<unsigned char>(lit.symbol));
            break;
        }
        case 1:
        {
            const auto &m = std::get<MatchEvent>(ev);
            int length = decodeLength(m.lenSym, m.lenExtraVal);
            int distance = decodeDistance(m.distSym, m.distExtraVal);
            tokens.push_back(Match{length, distance});
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
