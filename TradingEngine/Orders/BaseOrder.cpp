#include "BaseOrder.h"

BaseOrder::BaseOrder(int32_t userId, int64_t amount, int64_t filled) :
userId(userId),
amount(amount),
filled(filled) {
}

int64_t BaseOrder::GetAmount() const {
	return amount;
}

int32_t BaseOrder::GetUserId() const {
	return userId;
}

int64_t BaseOrder::GetRemaining() const {
	return (amount - filled);
}

int64_t BaseOrder::GetFilled() const {
	return filled;
}

void BaseOrder::AddToFill(int64_t fill) {
	filled += fill;
}

void BaseOrder::RemoveFromFill(int64_t fill) {
	filled -= fill;
}

void BaseOrder::SetId(int64_t orderId) {
	this->orderId = orderId;
}

int64_t BaseOrder::GetId() const {
	return orderId;
}

bool BaseOrder::operator==(const BaseOrder& order) const {
	return (userId == order.userId && orderId == order.orderId && amount == order.amount
	&& filled == order.filled);
}
