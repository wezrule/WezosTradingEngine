#include "StubListener.h"
#include "StubWallet.h"

#include <TradingEngine/Error.h>
#include <TradingEngine/Market.h>
#include <TradingEngine/Orders/MarketOrder.h>
#include <TradingEngine/Orders/OrderAction.h>
#include <TradingEngine/Orders/OrderContainer.h>
#include <TradingEngine/Units.h>
#include <TradingEngine/market_helper.h>
#include <gtest/gtest.h>
#include <memory>

MarketConfig createStubMarketConfig();

class EmptyMarketMakingMarketOrders : public ::testing::Test {
protected:
	StubWallet stubWallet;
	MarketWallets marketWallets;

	template <OrderAction Side>
	void check() {
		marketWallets = { &stubWallet, &stubWallet };

		Market market(std::make_unique<StubListener>(), { 4, 2 }, createStubMarketConfig());

		// Trying to trade at market price when there are no orders
		OrderContainer<MarketOrder> marketOrderContainer{ { 6, Units::ExToIn(25.0) }, 0 };
		ASSERT_THROW((market.NewProcess<Side>(marketOrderContainer, &marketWallets)), Error);
	}
};

TEST_F(EmptyMarketMakingMarketOrders, sell) {
	check<OrderAction::Sell>();
}

TEST_F(EmptyMarketMakingMarketOrders, buy) {
	check<OrderAction::Buy>();
}
