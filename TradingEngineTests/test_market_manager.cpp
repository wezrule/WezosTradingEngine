#include "StubListener.h"

#include <TradingEngine/CoinPair.h>
#include <TradingEngine/Error.h>
#include <TradingEngine/Market.h>
#include <TradingEngine/MarketManager.h>
#include <TradingEngine/market_helper.h>
#include <algorithm>
#include <gtest/gtest.h>
#include <memory>

MarketConfig createStubMarketConfig();

TEST(TestMarketManager, sameMarketId) {
	MarketManager marketManager;

	CoinPair coinPair{ 1, 2 };

	Market market{ std::make_unique<StubListener>(), coinPair, createStubMarketConfig() };

	marketManager.AddMarket(std::move(market));

	Market sameIdMarket{ std::make_unique<StubListener>(), coinPair, createStubMarketConfig() };
	ASSERT_THROW(marketManager.AddMarket(std::move(sameIdMarket)), Error);
}

TEST(TestMarketManager, getMarket) {
	MarketManager marketManager;
	CoinPair coinPair{ 1, 2 };

	// Get market with only one in there
	Market market{ std::make_unique<StubListener>(), coinPair, createStubMarketConfig() };
	marketManager.AddMarket(std::move(market));

	// Market has been moved.... so listener will be nulled out, not a valid comparison..
	ASSERT_EQ(market.GetCoinPair(), marketManager.GetMarket(coinPair)->GetCoinPair());

	// Get markets with more than one pair
	CoinPair anotherCoinPair{ 3, 2 };
	Market anotherMarket{ std::make_unique<StubListener>(), anotherCoinPair,
		createStubMarketConfig() };
	marketManager.AddMarket(std::move(anotherMarket));

	ASSERT_EQ(anotherMarket.GetCoinPair(),
	marketManager.GetMarket(anotherCoinPair)->GetCoinPair());

	// Recheck first market
	ASSERT_EQ(market.GetCoinPair(), marketManager.GetMarket(coinPair)->GetCoinPair());
}
