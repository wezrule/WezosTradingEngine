#include "MarketManager.h"

#include "CoinPair.h"
#include "Error.h"
#include "Orders/OrderAction.h"
#include "Orders/StopLimitOrder.h"
#include "market_helper.h"

#include <algorithm>
#include <iterator>
#include <utility>

void MarketManager::AddMarket(Market&& market) {
	auto& markets = marketsMap[market.GetCoinPair().GetBaseId()];
	auto lb = findLbMarketFromCoinId(markets, market.GetCoinPair().GetCoinId());
	if (lb != markets.end() && lb->GetCoinPair().GetCoinId() == market.GetCoinPair().GetCoinId()) {
		throw Error(Error::Type::MarketAlreadyExists);
	}

	lb = markets.insert(lb, std::move(market));
}

//
std::unordered_map<CoinPair, std::vector<int64_t>> MarketManager::CancelAll(int32_t userId, WalletManager& walletManager) {
	std::unordered_map<CoinPair, std::vector<int64_t>> cancelledOrderMap;

	for (auto& [baseId, markets] : marketsMap) {
		auto baseWallet = walletManager.GetWallet(baseId);
		baseWallet->GetAddress(userId)->SetInOrder(0);

		for (auto& market : markets) {
			auto coinWallet = walletManager.GetWallet(market.GetCoinPair().GetCoinId());
			coinWallet->GetAddress(userId)->SetInOrder(0);
			auto cancelledIds = market.CancelAll(userId);
			cancelledOrderMap.insert(std::make_pair(market.GetCoinPair(), std::move(cancelledIds)));
		}
	}
	return cancelledOrderMap;
}

// This cancels everyone's order in all markets
void MarketManager::CancelAll(WalletManager& walletManager) {
	for (auto& markets : marketsMap) {
		auto baseWallet = walletManager.GetWallet(markets.first);
		for (auto& address : baseWallet->GetAddresses()) {
			const_cast<Address&>(address).SetInOrder(0);
		}

		for (auto& market : markets.second) {
			auto coinWallet = walletManager.GetWallet(market.GetCoinPair().GetCoinId());
			for (auto& address : coinWallet->GetAddresses()) {
				const_cast<Address&>(address).SetInOrder(0);
			}

			market.CancelAll();
		}
	}
}

void MarketManager::SetFees(double feePercent) {
	for (auto& markets : marketsMap) {
		for (auto& market : markets.second) {
			market.SetFeePercentage(feePercent);
		}
	}
}

void MarketManager::SetMaxNumLimitOpenOrders(int32_t numOpenOrders) {
	for (auto& markets : marketsMap) {
		for (auto& market : markets.second) {
			market.SetMaxNumLimitOpenOrders(numOpenOrders);
		}
	}
}

void MarketManager::SetMaxNumStopLimitOpenOrders(int32_t numOpenOrders) {
	for (auto& markets : marketsMap) {
		for (auto& market : markets.second) {
			market.SetMaxNumStopLimitOpenOrders(numOpenOrders);
		}
	}
}

std::vector<Market>::iterator MarketManager::findLbMarketFromCoinId(
std::vector<Market>& markets, int32_t coinId) {
	auto lb = std::lower_bound(markets.begin(), markets.end(), coinId,
	[](const auto& market, int32_t coinId) {
		return (market.GetCoinPair().GetCoinId() < coinId);
	});

	return lb;
}

// Assumes that the market exists
std::vector<Market>::iterator MarketManager::GetMarket(const CoinPair& coinPair) {
	auto& markets = marketsMap[coinPair.GetBaseId()];
	return findLbMarketFromCoinId(markets, coinPair.GetCoinId());
}

const MarketsMap& MarketManager::GetMarkets() const {
	return marketsMap;
}

bool MarketManager::operator==(const MarketManager& marketManager) const {
	return (marketsMap == marketManager.marketsMap);
}
