#pragma once

#include "serializer_defines.h"

#include <functional>

SERIALIZE_HEADER(CoinPair)

class CoinPair {
public:
	SERIALIZE_FRIEND(CoinPair)

	CoinPair() = default; // For serializing
	CoinPair(int32_t coinId, int32_t baseId) :
	coinId(coinId),
	baseId(baseId) {
	}

	inline int32_t GetCoinId() const {
		return coinId;
	}

	inline int32_t GetBaseId() const {
		return baseId;
	}

	bool operator==(const CoinPair& coinPair) const {
		return (coinId == coinPair.coinId) && (baseId == coinPair.baseId);
	}

	bool operator!=(const CoinPair& coinPair) const {
		return !(*this == coinPair);
	}

	bool operator<(const CoinPair& coinPair) const {
		if (baseId == coinPair.baseId) {
			return coinId < coinPair.coinId;
		}

		return baseId < coinPair.baseId;
	}

private:
	int32_t coinId = -1;
	int32_t baseId = -1;
};

namespace std {
template <>
struct hash<CoinPair> {
	size_t operator()(const CoinPair& coinPair) const noexcept {
		return coinPair.GetCoinId();
	}
};
}
