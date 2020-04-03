#include "MarketOrder.h"

#include "../Error.h"

MarketOrder::MarketOrder(int32_t userId, int64_t amount) :
BaseOrder(userId, amount, 0) {
}

int64_t MarketOrder::GetPrice() const {
	throw Error(Error::Type::Internal, "Should never be called");
}
