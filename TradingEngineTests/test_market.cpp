#include "StubListener.h"

#include <TradingEngine/CoinPair.h>
#include <TradingEngine/Market.h>
#include <TradingEngine/market_helper.h>
#include <gtest/gtest.h>
#include <memory>

MarketConfig createStubMarketConfig();

TEST(TestMarket, defaults) {
	CoinPair coinPair{ 4, 2 };
	Market market(std::make_unique<StubListener>(), coinPair, createStubMarketConfig());

	ASSERT_EQ(market.GetBuyLimitOrderMap().size(), 0u);
	ASSERT_EQ(market.GetSellLimitOrderMap().size(), 0u);
	ASSERT_EQ(market.GetBuyStopLimitOrderMap().size(), 0u);
	ASSERT_EQ(market.GetSellStopLimitOrderMap().size(), 0u);
	ASSERT_EQ(market.GetCoinPair(), coinPair);
}
