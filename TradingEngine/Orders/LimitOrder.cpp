#include "LimitOrder.h"

LimitOrder::LimitOrder(int32_t userId, int64_t amount, int64_t filled) :
BaseOrder(userId, amount, filled) {
}

bool LimitOrder::operator==(const LimitOrder& order) const {
	return (BaseOrder::operator==(order));
}
