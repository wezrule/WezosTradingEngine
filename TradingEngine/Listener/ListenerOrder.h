#pragma once

#include "../Fee.h"
#include "../Orders/LimitOrder.h"
#include "../Orders/MarketOrder.h"
#include "../Orders/OrderContainer.h"
#include "../Orders/OrderType.h"
#include "../Orders/StopLimitOrder.h"

#include <cstdint>

template <typename Order>
class OrderContainer;

class ListenerOrder {
public:
	ListenerOrder() = default;
	ListenerOrder(int64_t tradeId, int64_t id1, int64_t id2, int64_t amount, int64_t filled,
	int64_t price, int64_t actualPrice, OrderType type);

	ListenerOrder(int64_t id1, int64_t id2, int64_t amount, int64_t filled, int64_t price,
	int64_t actualPrice, OrderType type);

	int64_t tradeId = -1;
	union {
		int64_t orderId = -1;
		int64_t buyOrderId;
		int64_t stopLimitId;
	};

	union {
		int32_t userId = -1;
		int64_t sellOrderId; // For a trade use this
		int64_t triggeredOrderId; // The order id which triggered the stop order
	};

	int64_t amount = 0;
	int64_t filled = 0;
	int64_t price = 0;
	int64_t actualPrice = 0;
	OrderType orderType;

	Fee fees;

	bool operator==(const ListenerOrder& listenerOrder) const;
};

class LimitOrder;
class MarketOrder;
class StopLimitOrder;

template <class T>
ListenerOrder ConvertToListenerOrder(const OrderContainer<T>& order);
template <>
ListenerOrder ConvertToListenerOrder(const OrderContainer<LimitOrder>& limitOrder);
template <>
ListenerOrder ConvertToListenerOrder(const OrderContainer<MarketOrder>& marketOrder);
template <>
ListenerOrder ConvertToListenerOrder(const OrderContainer<StopLimitOrder>& stopLimitOrder);
