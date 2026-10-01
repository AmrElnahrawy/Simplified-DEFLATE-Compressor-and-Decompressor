#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <variant>
#include <vector>
#include <queue>

#include "huffman.hpp"


void huffman::calculate_frequencies(const std::vector<Event> &events) {
        for (const auto& ev : events) {
            switch (ev.index()) {
                case 0: { 
                    const auto& l = std::get<LiteralEvent>(ev);
                    literal_length_freqs[l.symbol]++;
                    break;
                }
                case 1: { 
                    const auto& m = std::get<MatchEvent>(ev);
                    literal_length_freqs[m.lenSym]++;
                    distance_freqs[m.distSym]++;
                    break;
                }
                case 2: {
                    const auto& e = std::get<EndEvent>(ev);
                    literal_length_freqs[e.symbol]++;
                    break;
                }
            }
        }
    }

huffmanNode* huffman::huffman_tree(std::vector<uint32_t> freqs) {
        auto comp = [](const huffmanNode* a, const huffmanNode* b) { 
            return *b < *a; 
        };
        std::priority_queue<huffmanNode*, std::vector<huffmanNode*>, decltype(comp)> Nodes(comp);

        for (uint16_t i = 0; i < freqs.size(); i++) {
            if (freqs[i] != 0)
                Nodes.push(new huffmanNode(i, freqs[i], i));
        }

        if (Nodes.size() == 0) {
            return nullptr;
        }

        if (Nodes.size() == 1) {
            huffmanNode* dummy_root = new huffmanNode(Nodes.top()->symbol,Nodes.top()->freq,Nodes.top()->min_symbol);  
            dummy_root->left = Nodes.top();
            return dummy_root;
        }

        while (Nodes.size() > 1) {
            huffmanNode* left = Nodes.top();
            Nodes.pop();
            huffmanNode* right = Nodes.top();
            Nodes.pop();

            huffmanNode* parent = new huffmanNode(0, left->freq + right->freq, std::min(left->min_symbol, right->min_symbol));
            parent->left = left;
            parent->right = right;

            Nodes.push(parent);
        }

        return Nodes.top();
    }

void huffman::count_lengths(huffmanNode* root, uint32_t depth, std::vector<uint32_t>& x_code_length) {
        if (root == nullptr)
            return;
        
        if (root->left == nullptr && root->right == nullptr) {
            x_code_length[root->symbol] = depth;
            return;
        }
        if (root->left) {
            count_lengths(root->left, depth + 1, x_code_length);
        }
        if (root->right) {
            count_lengths(root->right, depth + 1, x_code_length);
        }
        return;
    }

std::vector<uint32_t> huffman::canonical_codes(const std::vector<uint32_t>& lengths) {
        std::vector<uint32_t> count(16, 0), next_code(16, 0);
        for (uint8_t l : lengths)  {
            if (l > 15) {
                throw std::runtime_error("Code length longer than 15");
            }
            count[l]++;
        }
        count[0] = 0;

        uint32_t code = 0;
        for (int bits = 1; bits < 16; bits++) {
            code = (code + count[bits - 1]) << 1;
            next_code[bits] = code;
        }

        std::vector<uint32_t> codes(lengths.size(), 0);
        for (size_t s = 0; s < lengths.size(); s++)
            if (lengths[s] != 0)
                codes[s] = next_code[lengths[s]]++;
        return codes;
    }
    
void huffman::encode(const std::vector<Event>& events) {
    calculate_frequencies(events);

    huffmanNode* ll_root = huffman_tree(literal_length_freqs);
    huffmanNode* d_root  = huffman_tree(distance_freqs);
        
    count_lengths(ll_root, 0,ll_code_lengths);
    count_lengths(d_root, 0,d_code_lengths);  

    delete ll_root;
    delete d_root;

    ll_codes = canonical_codes(ll_code_lengths);
    d_codes  = canonical_codes(d_code_lengths);
}

const std::vector<uint32_t>& huffman::get_ll_codes() const { return ll_codes; }
const std::vector<uint32_t>& huffman::get_d_codes()  const { return d_codes; }

const std::vector<uint32_t>& huffman::get_ll_code_lengths() const { return ll_code_lengths; }
const std::vector<uint32_t>& huffman::get_d_code_lengths()  const { return d_code_lengths; }



