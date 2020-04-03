#pragma once

#include "../Orders/orders.h"

#include <stddef.h>

namespace ps {

// The size of the fixed size deque chunks.
// To work this out, TradingEngineUnitTests/test_order_allocators.cpp
// should give an idea
#ifdef __GNUG__
template <class Order>
constexpr size_t GetOrderDequeSize() {
	static_assert(IsLimitOrder_v<Order> || IsStopLimitOrder_v<Order>);

	if constexpr (IsLimitOrder_v<Order>) {
		return 16;
	} else {
		return 12;
	}
}
#endif
}
