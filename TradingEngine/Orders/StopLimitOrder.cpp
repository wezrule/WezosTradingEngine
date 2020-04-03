#include "StopLimitOrder.h"

// TODO, possible to remove filled? Should always be 0...
StopLimitOrder::StopLimitOrder(int32_t userId, int64_t amount, int64_t filled,
int64_t actualPrice) :
LimitOrder(userId, amount, 0),
actualPrice(actualPrice) {
}

int64_t StopLimitOrder::GetActualPrice() const {
	return actualPrice;
}

bool StopLimitOrder::operator==(const StopLimitOrder& stopLimitOrder) const {
	return (LimitOrder::operator==(stopLimitOrder) && actualPrice == stopLimitOrder.actualPrice);
}
