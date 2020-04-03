#pragma once

#include "../serializer_defines.h"
#include "BaseOrder.h"

#include <cstdint>

SERIALIZE_HEADER(LimitOrder)

class LimitOrder : public BaseOrder {
public:
	SERIALIZE_FRIEND(LimitOrder)

	LimitOrder() = default; // For serializing
	LimitOrder(int32_t userId, int64_t amount, int64_t filled);
	bool operator==(const LimitOrder& order) const;
};
