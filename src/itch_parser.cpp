#include "itch_parser.hpp"
#include "binary_reader.hpp"
#include "itch_types.hpp"

#include <iostream>
#include <stdexcept>

namespace itch {

void Parser::parse_message(
    const uint8_t* data,
    std::size_t length
) {
    if (length == 0)
        return;

    const char type = static_cast<char>(data[0]);

    switch (type) {

        case 'S':
            parse_system_event(data, length);
            break;

        case 'R':
            parse_stock_directory(data, length);
            break;

        case 'A':
            parse_add_order(data, length);
            break;

        case 'F':
            parse_add_order_mpid(data, length);
            break;

        case 'E':
            parse_order_executed(data, length);
            break;

        case 'C':
            parse_order_executed_price(data, length);
            break;

        case 'X':
            parse_order_cancel(data, length);
            break;

        case 'D':
            parse_order_delete(data, length);
            break;

        case 'U':
            parse_order_replace(data, length);
            break;

        default:
            break;
    }
}

//add individual private functions
// implement the const benchmark func here 

//system event
void Parser::parse_system_event(
    const uint8_t* data,
    std::size_t length
) {
    constexpr std::size_t MESSAGE_LENGTH = 12;

    if (length < MESSAGE_LENGTH)
        throw std::runtime_error("Invalid S message length");

    BinaryReader reader(data, length);


    SystemEvent message{
        .stock_locate = reader.read_u16_be(1),
        .tracking_number = reader.read_u16_be(3),
        .timestamp = reader.read_u48_be(5),
        .event_code = static_cast<char>(reader.read_u8(11))
    };

    // std::cout
    //     << "[S]"
    //     << " stock_locate=" << message.stock_locate
    //     << " tracking_number=" << message.tracking_number
    //     << " timestamp=" << message.timestamp
    //     << " event=" << message.event_code
    //     << '\n';
}

//stock directory
void Parser::parse_stock_directory(
    const uint8_t* data,
    std::size_t length
) {
    constexpr std::size_t MESSAGE_LENGTH = 39;

    if (length < MESSAGE_LENGTH)
        throw std::runtime_error("Invalid R message length");

    BinaryReader reader(data, length);

    StockDirectory message{
        .stock_locate = reader.read_u16_be(1),
        .tracking_number = reader.read_u16_be(3),
        .timestamp = reader.read_u48_be(5),

        .stock = {},
        
        .market_category =
            static_cast<char>(reader.read_u8(19)),

        .financial_status_indicator =
            static_cast<char>(reader.read_u8(20)),

        .round_lot_size =
            reader.read_u32_be(21),

        .round_lots_only =
            static_cast<char>(reader.read_u8(25)),

        .issue_classification =
            static_cast<char>(reader.read_u8(26)),

        .issue_subtype = {},

        .authenticity =
            static_cast<char>(reader.read_u8(29)),

        .short_sale_threshold_indicator =
            static_cast<char>(reader.read_u8(30)),

        .ipo_flag =
            static_cast<char>(reader.read_u8(31)),

        .luld_reference_price_tier =
            static_cast<char>(reader.read_u8(32)),

        .etp_flag =
            static_cast<char>(reader.read_u8(33)),

        .etp_leverage_factor =
            reader.read_u32_be(34),

        .inverse_indicator =
            static_cast<char>(reader.read_u8(38))
    };

    reader.read_bytes(
        11,
        message.stock,
        8
    );

    reader.read_bytes(
        27,
        message.issue_subtype,
        2
    );

    // std::cout
    //     << "[R]"
    //     << " stock_locate=" << message.stock_locate
    //     << " tracking_number=" << message.tracking_number
    //     << " timestamp=" << message.timestamp
    //     << " stock=";

    // std::cout.write(message.stock, 8);

    // std::cout
    //     << " market_category=" << message.market_category
    //     << " financial_status="
    //     << message.financial_status_indicator
    //     << " round_lot_size=" << message.round_lot_size
    //     << " round_lots_only=" << message.round_lots_only
    //     << " issue_classification="
    //     << message.issue_classification
    //     << " issue_subtype=";

    // std::cout.write(message.issue_subtype, 2);

    // std::cout
    //     << " authenticity=" << message.authenticity
    //     << " short_sale_threshold="
    //     << message.short_sale_threshold_indicator
    //     << " ipo_flag=" << message.ipo_flag
    //     << " luld_tier="
    //     << message.luld_reference_price_tier
    //     << " etp_flag=" << message.etp_flag
    //     << " etp_leverage_factor="
    //     << message.etp_leverage_factor
    //     << " inverse=" << message.inverse_indicator
    //     << '\n';
}

//add order
void Parser::parse_add_order(
    const uint8_t* data,
    std::size_t length
) {
    constexpr std::size_t MESSAGE_LENGTH = 36;

    if (length < MESSAGE_LENGTH)
        throw std::runtime_error("Invalid A message length");

    BinaryReader reader(data, length);

    AddOrder message{
        .stock_locate = reader.read_u16_be(1),
        .tracking_number = reader.read_u16_be(3),
        .timestamp = reader.read_u48_be(5),
        .order_reference = reader.read_u64_be(11),
        .buy_sell_indicator =
            static_cast<char>(reader.read_u8(19)),
        .shares = reader.read_u32_be(20),
        .stock = {},
        .price = reader.read_u32_be(32)
    };

    reader.read_bytes(24, message.stock, 8);

    // std::cout
    //     << "[A]"
    //     << " locate=" << message.stock_locate
    //     << " tracking_number=" << message.tracking_number
    //     << " timestamp=" << message.timestamp
    //     << " order=" << message.order_reference
    //     << " side=" << message.buy_sell_indicator
    //     << " shares=" << message.shares
    //     << " stock=";

    // std::cout.write(message.stock, 8);

    // std::cout
    //     << " price=" << message.price
    //     << '\n';
}

//add order with mpid
void Parser::parse_add_order_mpid(
    const uint8_t* data,
    std::size_t length
) {
    constexpr std::size_t MESSAGE_LENGTH = 40;

    if (length < MESSAGE_LENGTH)
        throw std::runtime_error("Invalid F message length");

    BinaryReader reader(data, length);

    AddOrderMPID message{
        .stock_locate = reader.read_u16_be(1),
        .tracking_number = reader.read_u16_be(3),
        .timestamp = reader.read_u48_be(5),
        .order_reference = reader.read_u64_be(11),
        .buy_sell_indicator =
            static_cast<char>(reader.read_u8(19)),
        .shares = reader.read_u32_be(20),
        .stock = {},
        .price = reader.read_u32_be(32),
        .attribution = {}
    };

    reader.read_bytes(24, message.stock, 8);
    reader.read_bytes(36, message.attribution, 4);

    // std::cout
    //     << "[F]"
    //     << " order=" << message.order_reference
    //     << " side=" << message.buy_sell_indicator
    //     << " shares=" << message.shares
    //     << " stock=";

    // std::cout.write(message.stock, 8);

    // std::cout
    //     << " price=" << message.price
    //     << " attribution=";

    // std::cout.write(message.attribution, 4);

    // std::cout << '\n';
}

//Order Executed
void Parser::parse_order_executed(
    const uint8_t* data,
    std::size_t length
) {
    constexpr std::size_t MESSAGE_LENGTH = 31;

    if (length < MESSAGE_LENGTH)
        throw std::runtime_error("Invalid E message length");

    BinaryReader reader(data, length);

    OrderExecuted message{
        .stock_locate = reader.read_u16_be(1),
        .tracking_number = reader.read_u16_be(3),
        .timestamp = reader.read_u48_be(5),
        .order_reference = reader.read_u64_be(11),
        .executed_shares = reader.read_u32_be(19),
        .match_number = reader.read_u64_be(23)
    };

    // std::cout
    //     << "[E]"
    //     << " order=" << message.order_reference
    //     << " executed=" << message.executed_shares
    //     << " match=" << message.match_number
    //     << '\n';
}

//Order Executed with Price
void Parser::parse_order_executed_price(
    const uint8_t* data,
    std::size_t length
) {
    constexpr std::size_t MESSAGE_LENGTH = 36;

    if (length < MESSAGE_LENGTH)
        throw std::runtime_error("Invalid C message length");

    BinaryReader reader(data, length);

    OrderExecutedWithPrice message{
        .stock_locate = reader.read_u16_be(1),
        .tracking_number = reader.read_u16_be(3),
        .timestamp = reader.read_u48_be(5),
        .order_reference = reader.read_u64_be(11),
        .executed_shares = reader.read_u32_be(19),
        .match_number = reader.read_u64_be(23),
        .printable = static_cast<char>(reader.read_u8(31)),
        .execution_price = reader.read_u32_be(32)
    };

    // std::cout
    //     << "[C]"
    //     << " order=" << message.order_reference
    //     << " executed=" << message.executed_shares
    //     << " match=" << message.match_number
    //     << " printable=" << message.printable
    //     << " price=" << message.execution_price
    //     << '\n';
}

//Cancel Order
void Parser::parse_order_cancel(
    const uint8_t* data,
    std::size_t length
) {
    constexpr std::size_t MESSAGE_LENGTH = 23;

    if (length < MESSAGE_LENGTH)
        throw std::runtime_error("Invalid X message length");

    BinaryReader reader(data, length);

    OrderCancel message{
        .stock_locate = reader.read_u16_be(1),
        .tracking_number = reader.read_u16_be(3),
        .timestamp = reader.read_u48_be(5),
        .order_reference = reader.read_u64_be(11),
        .cancelled_shares = reader.read_u32_be(19)
    };

    // std::cout
    //     << "[X]"
    //     << " order=" << message.order_reference
    //     << " canceled=" << message.cancelled_shares
    //     << '\n';
}

//order delete
void Parser::parse_order_delete(
    const uint8_t* data,
    std::size_t length
) {
    constexpr std::size_t MESSAGE_LENGTH = 19;

    if (length < MESSAGE_LENGTH)
        throw std::runtime_error("Invalid D message length");

    BinaryReader reader(data, length);

    OrderDelete message{
        .stock_locate = reader.read_u16_be(1),
        .tracking_number = reader.read_u16_be(3),
        .timestamp = reader.read_u48_be(5),
        .order_reference = reader.read_u64_be(11)
    };

    // std::cout
    //     << "[D]"
    //     << " order=" << message.order_reference
    //     << '\n';
}

//order replace
void Parser::parse_order_replace(
    const uint8_t* data,
    std::size_t length
) {
    constexpr std::size_t MESSAGE_LENGTH = 35;

    if (length < MESSAGE_LENGTH)
        throw std::runtime_error("Invalid U message length");

    BinaryReader reader(data, length);

    OrderReplace message{
        .stock_locate = reader.read_u16_be(1),
        .tracking_number = reader.read_u16_be(3),
        .timestamp = reader.read_u48_be(5),
        .original_order_reference = reader.read_u64_be(11),
        .new_order_reference = reader.read_u64_be(19),
        .shares = reader.read_u32_be(27),
        .price = reader.read_u32_be(31)
    };

    // std::cout
    //     << "[U]"
    //     << " old_order=" << message.original_order_reference
    //     << " new_order=" << message.new_order_reference
    //     << " shares=" << message.shares
    //     << " price=" << message.price
    //     << '\n';
}
}

