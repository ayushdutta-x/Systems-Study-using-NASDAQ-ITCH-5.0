#pragma once

#include <cstddef>
#include <cstdint>

namespace itch {

class BinaryReader {
public:
    BinaryReader(const uint8_t* data, std::size_t size_)
        : data_(data), size_(size_) {}

    uint8_t read_u8(std::size_t offset) const;
    uint16_t read_u16_be(std::size_t offset) const;
    uint32_t read_u32_be(std::size_t offset) const;
    uint64_t read_u48_be(std::size_t offset) const;
    uint64_t read_u64_be(std::size_t offset) const;

    void read_bytes(
        std::size_t offset,
        void* destination,
        std::size_t length
    ) const;

private:
    const uint8_t* data_;
    std::size_t size_;
};

}


