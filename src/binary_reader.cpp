#include "binary_reader.hpp"

#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace itch {

uint8_t BinaryReader::read_u8(std::size_t offset) const {
    if (offset + 1 > size_)
        throw std::out_of_range("read_u8: offset out of range");

    return data_[offset];
}

uint16_t BinaryReader::read_u16_be(std::size_t offset) const {
    if (offset + 2 > size_)
        throw std::out_of_range("read_u16_be: out of range");

    return (static_cast<uint16_t>(data_[offset]) << 8) |
           static_cast<uint16_t>(data_[offset + 1]);
}

uint32_t BinaryReader::read_u32_be(std::size_t offset) const {
    if (offset + 4 > size_)
        throw std::out_of_range("read_u32_be: out of range");

    return (static_cast<uint32_t>(data_[offset]) << 24) |
           (static_cast<uint32_t>(data_[offset + 1]) << 16) |
           (static_cast<uint32_t>(data_[offset + 2]) << 8) |
           static_cast<uint32_t>(data_[offset + 3]);
}

uint64_t BinaryReader::read_u48_be(std::size_t offset) const {
    if (offset + 6 > size_)
        throw std::out_of_range("read_u48_be: out of range");

    return (static_cast<uint64_t>(data_[offset]) << 40) |
        (static_cast<uint64_t>(data_[offset + 1]) << 32) |
        (static_cast<uint64_t>(data_[offset + 2]) << 24) |
        (static_cast<uint64_t>(data_[offset + 3]) << 16) |
        (static_cast<uint64_t>(data_[offset + 4]) << 8) |
        static_cast<uint64_t>(data_[offset + 5]);

}

uint64_t BinaryReader::read_u64_be(std::size_t offset) const {
    if (offset + 8 > size_)
        throw std::out_of_range("read_u64_be: out of range");

    return (static_cast<uint64_t>(data_[offset]) << 56) |
    (static_cast<uint64_t>(data_[offset + 1]) << 48) |
    (static_cast<uint64_t>(data_[offset + 2]) << 40) |
    (static_cast<uint64_t>(data_[offset + 3]) << 32) |
    (static_cast<uint64_t>(data_[offset + 4]) << 24) |
    (static_cast<uint64_t>(data_[offset + 5]) << 16) |
    (static_cast<uint64_t>(data_[offset + 6]) << 8) |
    static_cast<uint64_t>(data_[offset + 7]);
}

void BinaryReader::read_bytes(
    std::size_t offset,
    void* destination,
    std::size_t length
) const {
    if (offset + length > size_)
        throw std::out_of_range("read_bytes");

    std::memcpy(destination, data_ + offset, length);
}

}