#include "container.hpp"

#include <stdexcept>
#include <algorithm>

#include "bitio.hpp"
#include "lz77.hpp"
#include "symbols.hpp"
#include "huffman.hpp"

namespace sdfl {
    namespace {
    
        constexpr int LLS = 286;
        constexpr int DS  = 30;
        
        constexpr int LEN_EXTRA[29] = {
            0,0,0,0,0,0,0,0, 1,1,1,1, 2,2,2,2, 3,3,3,3, 4,4,4,4, 5,5,5,5, 0
        };
        constexpr int DIST_EXTRA[30] = {
            0,0,0,0, 1,1, 2,2, 3,3, 4,4, 5,5, 6,6, 7,7, 8,8, 9,9, 10,10, 11,11, 12,12, 13,13
        };

        uint32_t bit_width(const std::vector<uint32_t>& lengths) {
            uint32_t m = *std::max_element(lengths.begin(), lengths.end());
            uint32_t bw = 0;
            while (m) { ++bw; m >>= 1; }
            return bw;
        }

        void put_bits(BitWriter& w, uint32_t value, uint32_t count) {
            if (count) w.write_bits(value, count);
        }

        uint32_t get_bits(BitReader& r, uint32_t count) {
            return count ? r.read_bits(count) : 0;
        }

        struct CanonicalDecoder {
            static constexpr int MAXB = 15;
            uint32_t count[MAXB + 1] = {0};
            std::vector<uint16_t> sorted;
        
            explicit CanonicalDecoder(const std::vector<uint32_t>& lengths) {
                for (uint32_t l : lengths) {
                    if (l > MAXB) throw std::runtime_error("invalid code length");
                    count[l]++;
                }
                count[0] = 0;
                for (int l = 1; l <= MAXB; ++l)
                    for (size_t s = 0; s < lengths.size(); ++s)
                        if (lengths[s] == (uint32_t)l) sorted.push_back((uint16_t)s);
            }
        
            uint16_t decode(BitReader& r) const {
                int code = 0, first = 0, index = 0;
                for (int len = 1; len <= MAXB; ++len) {
                    if (r.eof()) throw std::runtime_error("unexpected end of data");
                    code |= (int)r.read_bit();
                    int c = (int)count[len];
                    if (code - c < first)
                        return sorted[index + (code - first)];
                    index += c;
                    first += c;
                    first <<= 1;
                    code <<= 1;
                }
                throw std::runtime_error("invalid Huffman code");
            }
        };

    } // namespace

    std::vector<uint8_t> compress(const std::vector<uint8_t>& input) {
        // Stage 1 + 2
        std::vector<Token> tokens = LZ77::encode(input);
        std::vector<Event> events = Symbols::encode(tokens);
    
        // Stage 3
        huffman h;
        h.encode(events);
        const auto& llLen = h.get_ll_code_lengths();
        const auto& dLen  = h.get_d_code_lengths();
        const auto& llCode = h.get_ll_codes();
        const auto& dCode  = h.get_d_codes();
    
        // Stage 4: header
        BitWriter w;
        uint32_t litBW  = bit_width(llLen);
        uint32_t distBW = bit_width(dLen);
        put_bits(w, litBW, 4);
        put_bits(w, distBW, 4);
        for (int i = 0; i < LLS; ++i) put_bits(w, llLen[i], litBW);
        for (int i = 0; i < DS;  ++i) put_bits(w, dLen[i], distBW);
    
        // Stage 4: payload
        for (const Event& ev : events) {
            if (auto* lit = std::get_if<LiteralEvent>(&ev)) {
                put_bits(w, llCode[lit->symbol], llLen[lit->symbol]);
            } else if (auto* m = std::get_if<MatchEvent>(&ev)) {
                put_bits(w, llCode[m->lenSym], llLen[m->lenSym]);
                put_bits(w, m->lenExtraVal, m->lenExtraBits);
                put_bits(w, dCode[m->distSym], dLen[m->distSym]);
                put_bits(w, m->distExtraVal, m->distExtraBits);
            } else {
                const auto& e = std::get<EndEvent>(ev);
                put_bits(w, llCode[e.symbol], llLen[e.symbol]);
            }
        }
    
        w.flush(); // zero-pads the last byte
        return w.bytes();
    }
    
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& compressed) {
        if (compressed.empty()) throw std::runtime_error("empty compressed file");
    
        BitReader r(compressed);
    
        uint32_t litBW  = r.read_bits(4);
        uint32_t distBW = r.read_bits(4);
    
        std::vector<uint32_t> llLen(LLS), dLen(DS);
        for (int i = 0; i < LLS; ++i) llLen[i] = get_bits(r, litBW);
        for (int i = 0; i < DS;  ++i) dLen[i]  = get_bits(r, distBW);
    
        CanonicalDecoder llDec(llLen), dDec(dLen);
    
        std::vector<Event> events;
        while (true) {
            uint16_t sym = llDec.decode(r);
        
            if (sym < 256) {
                events.push_back(LiteralEvent{sym});
            } else if (sym == 256) {
                events.push_back(EndEvent{});
                break;
            } else {
                MatchEvent m{};
                m.lenSym = sym;
                m.lenExtraBits = (uint16_t)LEN_EXTRA[sym - 257];
                m.lenExtraVal  = (uint16_t)get_bits(r, m.lenExtraBits);
            
                if (dDec.sorted.empty())
                    throw std::runtime_error("match with empty distance table");
                m.distSym = dDec.decode(r);
                m.distExtraBits = (uint16_t)DIST_EXTRA[m.distSym];
                m.distExtraVal  = (uint16_t)get_bits(r, m.distExtraBits);
            
                events.push_back(m);
            }
        }
    
        // Stage 2 inverse, then Stage 1 inverse (byte-by-byte copy for overlaps)
        std::vector<Token> tokens = Symbols::decode(events);
        return LZ77::decode(tokens);
    }
}   