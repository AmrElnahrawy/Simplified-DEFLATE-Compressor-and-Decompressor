#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <vector>

#include "container.hpp"

 
namespace {
    std::vector<uint8_t> readFile(const std::string& path) {
        std::ifstream in(path, std::ios::binary);
        if (!in) {
            throw std::runtime_error("cannot open file: " + path);
        }
        return std::vector<uint8_t>((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    }
    
    void writeFile(const std::string& path, const std::vector<uint8_t>& data) {
        std::ofstream out(path, std::ios::binary);
        if (!out) {
            throw std::runtime_error("cannot open output file: " + path);
        }
        out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    }
    
    void printUsage(const char* argv0) {
        std::cerr << "Usage:\n"
                  << "  " << argv0 << " -c <file>         compress <file> into <file>.sdfl\n"
                  << "  " << argv0 << " -d <file>.sdfl    decompress into <file>\n";
    }
    
    void printUsage(const char* argv0) {
        std::cerr << "Usage:\n"
              << "  " << argv0 << " -c <file>         compress <file> into <file>.sdfl\n"
              << "  " << argv0 << " -d <file>.sdfl    decompress into <file>\n";
    }
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        printUsage(argv[0]);
        return 1;
    }

    const std::string mode = argv[1];
    const std::string filename = argv[2];

    try {
        if (mode == "-c") {

        } else if (mode == "-d") {
            
        } else {
            printUsage(argv[0]);
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}