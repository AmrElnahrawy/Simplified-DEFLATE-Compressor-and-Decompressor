#include <cstdint>
#include <vector>
#include <unordered_map>
#include <variant>

#include "types.hpp"
#include "lz77.hpp"

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
    static std::vector<Token> encode(const std::vector<unsigned char> &data) {
        std::vector<Token> tokens;
        std::unordered_map<uint32_t, RingBuffer> table;
        
        size_t i = 0;
        while (i < data.size()) {
            if (i + MIN_MATCH > data.size()) {
                tokens.push_back(data[i]);
                i++;
                continue;
            }

            uint32_t key = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
            
            int best_candidate_len = 0;
            int best_candidate_dist = 0;

            if (table.find(key) != table.end()) {
                RingBuffer& canditates = table[key];
                int count = canditates.indices.size();
                
                for (int j = 0; j < count; j++) {
                    int index = (canditates.head + count - 1 - j) % count;
                    int candidate_i = canditates.indices[index];

                    if (i - candidate_i > WINDOW_SIZE)
                        break;

                    int length = 0;
                    while (i + length < data.size() && length < MAX_MATCH && data[i + length] == data[candidate_i + length]) {
                        length++;
                    }

                    if (length > best_candidate_len) {
                        best_candidate_len = length;
                        best_candidate_dist = i - candidate_i;
                        if (best_candidate_len == MAX_MATCH)
                            break;
                    }
                }
            }

            if (best_candidate_len >= MIN_MATCH) {
                tokens.push_back(Match {best_candidate_len, best_candidate_dist});
                int end = i + best_candidate_len;
                while (i < end) {
                    if (i + MIN_MATCH <= data.size()) {
                        int k = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
                        table[k].push(i);
                    }
                    i++;
                }
            } else {
                tokens.push_back(data[i]);
                table[key].push(i);
                i++;
            }
        }
        return tokens;
    }

    static std::vector<unsigned char> decode(const std::vector<Token> &tokens) {
        std::vector<unsigned char> decoded_data;
        for (int i = 0; i < tokens.size(); i++) {
            if (std::holds_alternative<unsigned char>(tokens[i])) {
                decoded_data.push_back(std::get<unsigned char>(tokens[i]));
            } else {
                int counter = 0;
                int match_length = std::get<Match>(tokens[i]).length;
                int match_distance = std::get<Match>(tokens[i]).distance;
                while (counter++ < match_length) {
                    decoded_data.push_back(decoded_data[decoded_data.size() - match_distance]);
                }
            }
        }
        return decoded_data;
    };
};


