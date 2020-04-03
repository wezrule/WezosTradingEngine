#include "Operation.h"

#include "../Orders/OrderType.h"

Operation::Operation(Type type, int64_t tradeId, int64_t buyOrderId, int64_t sellOrderId,
int64_t amount, int64_t price, const Fee& fees) :
type(type),
listenerOrder(
// Limit is just a dummy value...
{ tradeId, buyOrderId, sellOrderId, amount, 0, price, -1, OrderType::Limit }) {
	listenerOrder.fees = fees;
}

void Operation::SetType(Type type) {
	this->type = type;
}

void Operation::SetAction(OrderAction action) {
	this->action = action;
}

void Operation::SetOrder(const ListenerOrder& listenerOrder) {
	this->listenerOrder = listenerOrder;
}

bool Operation::operator==(const Operation& operation) const {
	return type == operation.type && listenerOrder == operation.listenerOrder
	&& action == operation.action;
}
