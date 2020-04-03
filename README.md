###WezosTradingEngine

This was a made a couple years earlier than the commits suggest. Wanted to make a order matching engine for a cryptocurrency exchange as fast as possible.

Capable of processing over 1 million orders a second.

Supports Market/Limit/Stop-Limit orders

Very little branches used and memory allocations made. Custom block allocators are used for the orders.

Dependencies are boost headers and Boost.serialization library

Build with cmake
