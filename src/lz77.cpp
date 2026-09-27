#include <cstdint>
#include <vector>
#include <unordered_map>
#include <variant>

#include "lz77.hpp"

struct Match {
    int length;     // copy - past  [abc <--l--> abc]
    int distance;   // go back      [x <--d-- x]
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

    std::vector<Token> encode() {
        std::vector<Token> tokens_list;
        std::unordered_map<uint32_t, RingBuffer> table;
        
        int i = 0;
        while (i < data.size()) {
            if (i + MIN_MATCH > data.size()) {
                tokens_list.push_back(data[i]);
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
                tokens_list.push_back(Match {best_candidate_len, best_candidate_dist});
                int end = i + best_candidate_len;
                while (i < end) {
                    if (i + MIN_MATCH <= data.size()) {
                        int k = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
                        table[k].push(i);
                    }
                    i++;
                }
            } else {
                tokens_list.push_back(data[i]);
                table[key].push(i);
                i++;
            }
        }
        return tokens_list;
    }

    std::vector<unsigned char> decode(const std::vector<Token> &tokens_list) {
        std::vector<unsigned char> decoded_data;
        for (int i = 0; i < tokens_list.size(); i++) {
            if (std::holds_alternative<unsigned char>(tokens_list[i])) {
                decoded_data.push_back(std::get<unsigned char>(tokens_list[i]));
            } else {
                int counter = 0;
                int match_length = std::get<Match>(tokens_list[i]).length;
                int match_distance = std::get<Match>(tokens_list[i]).distance;
                while (counter++ < match_length) {
                    decoded_data.push_back(decoded_data[decoded_data.size() - match_distance]);
                }
            }
        }
        return decoded_data;
    };
};


