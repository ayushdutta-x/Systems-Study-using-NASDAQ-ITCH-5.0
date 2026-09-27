#include "binary_reader.hpp"
#include "itch_parser.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

int main() {

    // ------------------------------------------------------------
    // Open ITCH input file
    // ------------------------------------------------------------

    std::ifstream file(
        "data/sample.itch",
        std::ios::binary
    );

    if (!file) {
        std::cerr << "Failed to open sample.itch\n";
        return 1;
    }

    // ------------------------------------------------------------
    // Determine file size
    // ------------------------------------------------------------

    file.seekg(0, std::ios::end);
    std::size_t file_size = static_cast<std::size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    // ------------------------------------------------------------
    // Load entire file into memory
    // ------------------------------------------------------------

    std::vector<uint8_t> buffer(file_size);

    file.read(
        reinterpret_cast<char*>(buffer.data()),
        static_cast<std::streamsize>(file_size)
    );

    // ------------------------------------------------------------
    // Initialize parser and benchmark state
    // ------------------------------------------------------------

    itch::Parser parser;

    std::size_t offset = 0;
    std::size_t message_count = 0;

    std::array<std::size_t, 256> message_counts{};

    // ------------------------------------------------------------
    // Start benchmark
    // ------------------------------------------------------------

    auto start = std::chrono::steady_clock::now();

    // ------------------------------------------------------------
    // Parse ITCH messages
    //
    // Format:
    // [2-byte message length][message payload]
    // ------------------------------------------------------------

    while (offset < buffer.size()) {

        // Need at least 2 bytes for the length prefix.
        if (offset + 2 > buffer.size()) {
            break;
        }

        itch::BinaryReader reader(
            buffer.data() + offset,
            buffer.size() - offset
        );

        uint16_t message_length = reader.read_u16_be(0);

        offset += 2;

        // Check whether the complete message is present.
        if (offset + message_length > buffer.size()) {
            break;
        }

        // Record message type.
        message_counts[buffer[offset]]++;

        // Parse message payload.
        parser.parse_message(
            buffer.data() + offset,
            message_length
        );

        offset += message_length;
        ++message_count;
    }

    // ------------------------------------------------------------
    // End benchmark
    // ------------------------------------------------------------

    auto end = std::chrono::steady_clock::now();

    std::chrono::duration<double> elapsed = end - start;

    // ------------------------------------------------------------
    // Benchmark results
    // ------------------------------------------------------------

    std::cout << "\n===== BENCHMARK =====\n";

    std::cout << "Complete messages: "
              << message_count
              << '\n';

    std::cout << "Elapsed time: "
              << elapsed.count()
              << " s\n";

    std::cout << "Messages/sec: "
              << static_cast<double>(message_count) / elapsed.count()
              << '\n';

    std::cout << "Throughput: "
              << (static_cast<double>(buffer.size()) / elapsed.count())
                   / (1024.0 * 1024.0)
              << " MiB/s\n";

    // ------------------------------------------------------------
    // Message distribution
    // ------------------------------------------------------------

    std::cout << "\n===== MESSAGE COUNTS =====\n";

    for (std::size_t i = 0; i < message_counts.size(); ++i) {

        if (message_counts[i] > 0) {

            std::cout
                << static_cast<char>(i)
                << ": "
                << message_counts[i]
                << '\n';
        }
    }

    return 0;
}