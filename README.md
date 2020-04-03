## WezosTradingEngine

This project was made a couple years earlier (early 2018) than the commits suggest. I wanted to make a cryptocurrency exchange order matching engine as fast as possible. It's capable of processing over 1 million orders a second of random Market/Limit & Stop-Limit orders. For limit orders it only supports the Good-Till cancelled time in force policy. Rather than let it fester I decided to share it, I didn't spend a lot of time cleaning it up, so I would advise only using it for learning purposes. 

In addition it allows simulating orders without affecting the main order book itself, prevents self-trading too. 

Very little branches used and memory allocations made (custom block allocators are used for the order book).

Dependencies are boost headers and Boost.serialization library. Can serialize all objects in memory to a file easily, for later inspection and deserialization.

Build with `cmake`

Currently only tested with gcc 7, it will require some changes to TradingEngine\PlatformSpecific\allocator_constants.h to build on other platforms and probably some other changes.

This was only a portion of the overall system, if I have time I can get the other elements added.
