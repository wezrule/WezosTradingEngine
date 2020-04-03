#pragma once

#include "Market.h"
#include "WalletManager.h"
#include "serializer_defines.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

SERIALIZE_HEADER(MarketManager)

using MarketsMap = std::unordered_map<int32_t, std::vector<Market>>;

class MarketManager {
public:
	SERIALIZE_FRIEND(MarketManager)

	MarketManager() = default; // For serializing
	void AddMarket(Market&& market);

	std::vector<Market>::iterator GetMarket(const CoinPair& coinPair);

	void SetFees(double feePercent);
	void SetMaxNumLimitOpenOrders(int32_t numOpenOrders);
	void SetMaxNumStopLimitOpenOrders(int32_t numOpenOrders);

	std::unordered_map<CoinPair, std::vector<int64_t>> CancelAll(int32_t userId, WalletManager& walletManager);
	void CancelAll(WalletManager& walletManager);

	bool operator==(const MarketManager& marketManager) const;

	// Just for testing
	const MarketsMap& GetMarkets() const;

private:
	// Map of base coin ids against a vector of markets for each coin pair sorted by coin id
	MarketsMap marketsMap;
	std::vector<Market>::iterator findLbMarketFromCoinId(std::vector<Market>& markets, int32_t coinId);
};
