#pragma once

#include "../serializer_defines.h"
#include "BaseOrder.h"

#include <cstdint>

SERIALIZE_HEADER(MarketOrder)

class MarketOrder : public BaseOrder {
public:
	SERIALIZE_FRIEND(MarketOrder)

	MarketOrder() = default; // For serializing
	MarketOrder(int32_t userId, int64_t amount);
	int64_t GetPrice() const;
};
