#include "StubListener.h"
#include "StubWallet.h"

#include <TradingEngine/Error.h>
#include <TradingEngine/Market.h>
#include <TradingEngine/Orders/OrderAction.h>
#include <TradingEngine/Orders/OrderContainer.h>
#include <TradingEngine/Orders/StopLimitOrder.h>
#include <TradingEngine/Units.h>
#include <TradingEngine/market_helper.h>
#include <gtest/gtest.h>
#include <memory>

MarketConfig createStubMarketConfig();

class OnlyStopOrders : public ::testing::Test {
protected:
	template <OrderAction Side>
	void check() {
		StubWallet stubWallet;
		MarketWallets marketWallets = { &stubWallet, &stubWallet };

		Market market(std::make_unique<StubListener>(), CoinPair{ 4, 2 },
		createStubMarketConfig());

		// Not allowed as there are no orders
		StopLimitOrder stopLimitOrder = { 6, Units::ExToIn(10.0), 0, Units::ExToIn(0.025) };
		OrderContainer stopLimitOrderContainer{ stopLimitOrder, Units::ExToIn(0.025) };
		ASSERT_THROW((market.NewProcess<Side>(stopLimitOrderContainer, &marketWallets)), Error);
	}
};

TEST_F(OnlyStopOrders, sell) {
	check<OrderAction::Sell>();
}

TEST_F(OnlyStopOrders, buy) {
	check<OrderAction::Buy>();
}
