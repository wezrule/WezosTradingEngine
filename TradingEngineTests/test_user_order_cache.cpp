#include "StubListener.h"
#include "StubWallet.h"

#include <TradingEngine/Market.h>
#include <TradingEngine/Orders/OrderAction.h>
#include <TradingEngine/Orders/OrderContainer.h>
#include <TradingEngine/Orders/StopLimitOrder.h>
#include <TradingEngine/Units.h>
#include <TradingEngine/market_helper.h>
#include <cstdint>
#include <gtest/gtest.h>
#include <memory>

MarketConfig createStubMarketConfig();

// All user's open orders are stored locally
class UserOrderCacheLimit : public ::testing::Test {
private:
	std::unique_ptr<Market> market;
	StubWallet stubWallet;
	MarketWallets marketWallets;
	int32_t userId = 6;

protected:
	template <OrderAction Side, OrderAction OtherSide>
	void SetUp(double stopLimitRate) {
		market = std::make_unique<Market>(std::make_unique<StubListener>(), CoinPair{ 4, 2 },
		createStubMarketConfig());
		marketWallets = { &stubWallet, &stubWallet };

		// Limit Order (same prices)
		LimitOrder limitOrder{ userId, Units::ExToIn(100.0), 0 };
		market->NewProcess<Side>(OrderContainer{ limitOrder, Units::ExToIn(0.4) }, &marketWallets);
		market->NewProcess<Side>(OrderContainer{ limitOrder, Units::ExToIn(0.4) }, &marketWallets);

		// Stop-Limit Order (same price)
		auto amount = Units::ExToIn(100.0);
		StopLimitOrder stopLimitOrder{ userId, amount, 0, Units::ExToIn(stopLimitRate) };
		OrderContainer stopLimitOrderContainer{ stopLimitOrder, Units::ExToIn(stopLimitRate) };
		market->NewProcess<OtherSide>(stopLimitOrderContainer, &marketWallets);
		market->NewProcess<OtherSide>(stopLimitOrderContainer, &marketWallets);
	}

	template <OrderAction Side, OrderAction OtherSide>
	void Check(double stopLimitRate) {
		auto& userLimitOrderCache = market->GetUserOrderCache<Side, LimitOrder>(userId);
		ASSERT_EQ(userLimitOrderCache.size(), 2u);
		ASSERT_EQ(userLimitOrderCache.front().price, Units::ExToIn(0.4));
		ASSERT_EQ(userLimitOrderCache.front().orderId, 1);
		ASSERT_EQ(userLimitOrderCache.back().orderId, 2);

		// Cancelling an order
		market->CancelOrder<Side, LimitOrder>(2, Units::ExToIn(0.4), &marketWallets);
		ASSERT_EQ(userLimitOrderCache.size(), 1u);
		ASSERT_EQ(userLimitOrderCache.back().orderId, 1);

		auto& userStopLimitOrderCache = market->GetUserOrderCache<OtherSide, StopLimitOrder>(
		userId);
		ASSERT_EQ(userStopLimitOrderCache.size(), 2u);
		ASSERT_EQ(userStopLimitOrderCache.front().price, Units::ExToIn(stopLimitRate));
		ASSERT_EQ(userStopLimitOrderCache.front().orderId, 3);
		ASSERT_EQ(userStopLimitOrderCache.back().orderId, 4);

		// Cancelling an order
		market->CancelOrder<OtherSide, StopLimitOrder>(3, Units::ExToIn(stopLimitRate),
		&marketWallets);
		ASSERT_EQ(userStopLimitOrderCache.size(), 1u);
		ASSERT_EQ(userStopLimitOrderCache.back().orderId, 4);
	}
};

TEST_F(UserOrderCacheLimit, sell) {
	SetUp<OrderAction::Sell, OrderAction::Buy>(0.5);
	Check<OrderAction::Sell, OrderAction::Buy>(0.5);
}

TEST_F(UserOrderCacheLimit, buy) {
	SetUp<OrderAction::Buy, OrderAction::Sell>(0.3);
	Check<OrderAction::Buy, OrderAction::Sell>(0.3);
}
