#pragma once

#include "../Orders/OrderAction.h"
#include "ListenerOrder.h"

#include <cstdint>

struct Fee;

struct Operation {
public:
	enum class Type;

	Type type;
	ListenerOrder listenerOrder;
	OrderAction action;

	enum class Type {
		Cancel,
		OrderFilled,
		NewTrade,
		NewOpenOrder,
		NewFilledOrder,
		PartialFill,
		StopLimitTriggered
	};

	Operation() = default;
	Operation(Type type, int64_t tradeId, int64_t buyOrderId, int64_t sellOrderId, int64_t amount,
	int64_t price, const Fee& fees);

	void SetType(Type type);
	void SetAction(OrderAction action);
	void SetOrder(const ListenerOrder& listenerOrder);
	bool operator==(const Operation& operation) const;
};
