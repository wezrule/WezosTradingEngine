#include "Listener.h"

#include "ListenerOrder.h"

void Listener::OrderFilled(int64_t id) {
	Operation operation;
	operation.SetType(Operation::Type::OrderFilled);
	operation.listenerOrder.orderId = id;
	operations.push_back(operation);
}

void Listener::NewOpenOrder(const ListenerOrder& order, OrderAction action) {
	operations.emplace_back(CreateOrder(Operation::Type::NewOpenOrder, order, action));
}

void Listener::NewTrade(int64_t tradeId, int64_t buyOrderId, int64_t sellOrderId,
int64_t amount, int64_t price, const Fee& fees) {
	// Saves a copy (many trades could be made, so should be efficiently created stored)
	operations.emplace_back(Operation::Type::NewTrade, tradeId, buyOrderId, sellOrderId, amount,
	price, fees);
}

void Listener::NewFilledOrder(const ListenerOrder& order, OrderAction action) {
	operations.emplace_back(CreateOrder(Operation::Type::NewFilledOrder, order, action));
}

void Listener::PartialFill(int64_t id, int64_t fill) {
	Operation operation;
	operation.SetType(Operation::Type::PartialFill);
	operation.listenerOrder.orderId = id;
	operation.listenerOrder.filled = fill;
	operations.push_back(operation);
}

void Listener::StopLimitTriggered(int64_t stopLimitId, int64_t triggeredOrderId) {
	Operation operation;
	operation.SetType(Operation::Type::StopLimitTriggered);
	operation.listenerOrder.stopLimitId = stopLimitId;
	operation.listenerOrder.triggeredOrderId = triggeredOrderId;
	operations.push_back(operation);
}

const std::vector<Operation>& Listener::GetOperations() const {
	return operations;
}

void Listener::AddOperation(const Operation& operation) {
	operations.push_back(operation);
}

void Listener::ClearOperations() {
	operations.clear();
}

bool Listener::Equals(const IListener& inListener) const {
	const auto& listener = dynamic_cast<const Listener&>(inListener);
	return (operations == listener.operations);
}

std::unique_ptr<IListener> Listener::Clone() const {
	return std::make_unique<Listener>();
}

Operation Listener::CreateOrder(Operation::Type operationType, const ListenerOrder& order,
OrderAction action) {
	Operation operation;
	operation.SetType(operationType);
	operation.SetOrder(order);
	operation.SetAction(action);
	return operation;
}
