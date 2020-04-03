#pragma once

#include <cstdint>

struct Fee {
	int64_t buyFee = -1;
	int64_t sellFee = -1;

	static int32_t ConvertToDivisibleFee(double feePercent) {
		auto fraction = feePercent / 100;
		return static_cast<int32_t>((1 / fraction) + 0.5);
	}

	static double ConvertToFeePercent(int32_t divisibleFee) {
		if (divisibleFee == 0) {
			return 0.0;
		}

		return (1.0 / divisibleFee) * 100;
	}

	inline bool operator==(const Fee& fees) const {
		return (buyFee == fees.buyFee && sellFee == fees.sellFee);
	}
};
