#pragma once

#include "Fee.h"

#include <cstdint>

struct SimulatorTrade {
	int32_t buyUserId;
	int32_t sellUserId;
	int64_t amount;
	int64_t price;
	Fee fees;

	SimulatorTrade(int32_t buyUserId, int32_t sellUserId, int64_t amount, int64_t price,
	const Fee& fees) :
	buyUserId(buyUserId),
	sellUserId(sellUserId),
	amount(amount),
	price(price),
	fees(fees) {
	}
};
