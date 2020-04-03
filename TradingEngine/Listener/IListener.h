#pragma once

#include "../Orders/OrderAction.h"
#include "Operation.h"

#include <cstdint>
#include <memory>
#include <vector>

struct Fee;

class IListener {
public:
	virtual ~IListener() = default;
	virtual void OrderFilled(int64_t id) = 0;
	virtual void NewOpenOrder(const ListenerOrder& order, OrderAction action) = 0;
	virtual void NewTrade(int64_t tradeId, int64_t buyOrderId, int64_t sellOrderId,
	int64_t amount, int64_t price, const Fee& fees)
	= 0;
	virtual void NewFilledOrder(const ListenerOrder& order, OrderAction action) = 0;
	virtual void PartialFill(int64_t id, int64_t amount) = 0;
	virtual void StopLimitTriggered(int64_t stopLimitId, int64_t triggeredOrderId) = 0;
	virtual bool Equals(const IListener& listener) const = 0;

	bool operator==(const IListener& listener) const {
		return Equals(listener);
	}

	virtual const std::vector<Operation>& GetOperations() const = 0;
	virtual void ClearOperations() = 0;
	virtual std::unique_ptr<IListener> Clone() const = 0;
};
