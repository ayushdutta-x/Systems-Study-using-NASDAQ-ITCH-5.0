This performance experimentation was conducted on a subset of the NASDAQ TotalView ITCH 5.0
You can find the full documentation --> r"/docs/NQTVITCHSpecification.pdf"

============================================================================================
The selected message types for this project are:

SYSTEM
  S  System Event

REFERENCE
  R  Stock Directory

ORDER LIFECYCLE
  A  Add Order
  F  Add Order with MPID Attribution

  E  Order Executed
  C  Order Executed with Price

  X  Order Cancel
  D  Order Delete
  U  Order Replace

==============================================================================================



## later
TRADE / VALIDATION
  P  Trade
  B  Broken Trade
##



# commands

xxd -g 1 data/sample.itch | head    --- view the data
xxd -p -c 16 data/sample.itch | head   --- full raw (first 16 bytes)


00000000: 00 0c 53 00 00 00 00 09 fa 4d 3a d2 6b 4f 00 27  ..S......M:.kO.' -- first row 
          00 0c 53 00 00 00 00 09 fa 4d 3a d2 6b 4f 00 27




//run commands

g++ -std=c++20 -Wall -Wextra -Iinclude src/*.cpp -o itch_parser    -- build executable
./itch_parser                                                      -- run executable