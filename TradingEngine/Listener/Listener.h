#pragma once

#include "../Orders/OrderAction.h"
#include "IListener.h"
#include "Operation.h"

#include <cstdint>
#include <memory>
#include <vector>

class ListenerOrder;

class Listener : public IListener {
public:
	void OrderFilled(int64_t id) override;
	void NewOpenOrder(const ListenerOrder& order, OrderAction action) override;
	void NewTrade(int64_t tradeId, int64_t buyOrderId, int64_t sellOrderId, int64_t amount,
	int64_t price, const Fee& fees) override;
	void NewFilledOrder(const ListenerOrder& order, OrderAction action) override;
	void PartialFill(int64_t id, int64_t amount) override;
	void StopLimitTriggered(int64_t stopLimitId, int64_t triggeredTradeId) override;
	const std::vector<Operation>& GetOperations() const override;
	void ClearOperations() override;
	bool Equals(const IListener& listener) const override;

	std::unique_ptr<IListener> Clone() const override;

	// Only for testing
	void AddOperation(const Operation& operation);

private:
	std::vector<Operation> operations;

	Operation CreateOrder(Operation::Type operationType, const ListenerOrder& order,
	OrderAction action);
};
