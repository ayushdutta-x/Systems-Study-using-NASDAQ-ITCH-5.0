#pragma once

#include "itch_types.hpp"

#include <cstddef>
#include <cstdint>

namespace itch {

class Parser {
public:
    void parse_message(const uint8_t* data, std::size_t length);
    uint64_t checksum() const{
        return checksum_;
    }

private:
    uint64_t checksum_ = 0;
    void parse_system_event(const uint8_t* data, std::size_t length);
    void parse_stock_directory(const uint8_t* data, std::size_t length);
    void parse_add_order(const uint8_t* data, std::size_t length);
    void parse_add_order_mpid(const uint8_t* data, std::size_t length);
    void parse_order_executed(const uint8_t* data, std::size_t length);
    void parse_order_executed_price(const uint8_t* data, std::size_t length);
    void parse_order_cancel(const uint8_t* data, std::size_t length);
    void parse_order_delete(const uint8_t* data, std::size_t length);
    void parse_order_replace(const uint8_t* data, std::size_t length);
};

}
