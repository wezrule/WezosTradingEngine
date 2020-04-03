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

class InvalidStopLimitPrice : public ::testing::Test {
protected:
	std::unique_ptr<Market> market;
	StubWallet stubWallet;
	MarketWallets marketWallets;

	template <OrderAction Side>
	void setup() {
		marketWallets = { &stubWallet, &stubWallet };

		this->market = std::make_unique<Market>(std::make_unique<StubListener>(), CoinPair{ 4, 2 },
		createStubMarketConfig());

		LimitOrder limitOrder{ 6, Units::ExToIn(10.0), 0 };
		OrderContainer orderContainer{ limitOrder, Units::ExToIn(0.025) };
		market->NewProcess<Side>(orderContainer, &marketWallets);
	}

	template <OrderAction Side>
	void check(double stopPrice) {
		// The stop rate passed in is greater than the current ask order when buying, and lower
		// than the lowest bid when selling
		StopLimitOrder stopLimitOrder{ 7, Units::ExToIn(10.0), 0, Units::ExToIn(stopPrice) };
		OrderContainer stopLimitOrderContainer{ stopLimitOrder, Units::ExToIn(stopPrice) };
		ASSERT_THROW(market->NewProcess<Side>(stopLimitOrderContainer, &marketWallets), Error);
	}
};

TEST_F(InvalidStopLimitPrice, sell) {
	setup<OrderAction::Sell>();
	check<OrderAction::Sell>(0.02);
}

TEST_F(InvalidStopLimitPrice, buy) {
	setup<OrderAction::Buy>();
	check<OrderAction::Buy>(0.04);
}
