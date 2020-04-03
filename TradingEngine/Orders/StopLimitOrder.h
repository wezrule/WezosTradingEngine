#pragma once

#include "../serializer_defines.h"
#include "LimitOrder.h"

#include <cstdint>

SERIALIZE_HEADER(StopLimitOrder)

class StopLimitOrder : public LimitOrder {
public:
	SERIALIZE_FRIEND(StopLimitOrder)

	StopLimitOrder() = default; // For serializing
	StopLimitOrder(int32_t userId, int64_t amount, int64_t filled, int64_t actualPrice);
	int64_t GetActualPrice() const;
	bool operator==(const StopLimitOrder& stopLimitOrder) const;

private:
	int64_t actualPrice = 0; // The price as a limit order when triggered
};
