#include "StubListener.h"
#include "StubWallet.h"

#include <TradingEngine/Error.h>
#include <TradingEngine/Market.h>
#include <TradingEngine/Orders/LimitOrder.h>
#include <TradingEngine/Orders/MarketOrder.h>
#include <TradingEngine/Orders/OrderAction.h>
#include <TradingEngine/Orders/OrderContainer.h>
#include <TradingEngine/Units.h>
#include <TradingEngine/market_helper.h>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

MarketConfig createStubMarketConfig();

class OnlyLimitOrdersMarketTrade : public ::testing::Test {
protected:
	std::unique_ptr<Market> market;
	StubWallet stubWallet;
	MarketWallets marketWallets;

	template <OrderAction Side>
	void setup(const std::vector<double>& rates) {
		marketWallets = { &stubWallet, &stubWallet };

		this->market = std::make_unique<Market>(std::make_unique<StubListener>(), CoinPair{ 4, 2 },
		createStubMarketConfig());

		LimitOrder limitOrder{ 7, Units::ExToIn(100.0), 0 };
		OrderContainer orderContainer{ limitOrder, Units::ExToIn(rates[0]) };
		market->NewProcess<Side>(orderContainer, &marketWallets);
		orderContainer = { limitOrder, Units::ExToIn(rates[1]) };
		market->NewProcess<Side>(orderContainer, &marketWallets);
		orderContainer = { limitOrder, Units::ExToIn(rates[2]) };
		market->NewProcess<Side>(orderContainer, &marketWallets);
		limitOrder = { 7, Units::ExToIn(50.0), 0 };
		orderContainer = { limitOrder, Units::ExToIn(rates[3]) };
		market->NewProcess<Side>(orderContainer, &marketWallets);
	}

	template <OrderAction OtherSide, class Comp>
	void check(const OrderMap<LimitOrder, Comp>& limitOrderMap) {
		// Market trade a bit into 1 order
		market->NewProcess<OtherSide>(OrderContainer<MarketOrder>{ { 6, Units::ExToIn(25.0) }, 0 },
		&marketWallets);
		auto limitOrders = Flatten(limitOrderMap);

		ASSERT_DOUBLE_EQ(limitOrders.front().GetRemaining(), Units::ExToIn(25.0));

		// Market trade exactly 1 order
		OrderContainer<MarketOrder> marketOrderContainer{ { 6, Units::ExToIn(25.0) }, 0 };
		market->NewProcess<OtherSide>(marketOrderContainer, &marketWallets);
		limitOrders = Flatten(limitOrderMap);

		ASSERT_EQ(limitOrders.size(), 3u);
		ASSERT_DOUBLE_EQ(limitOrders.front().GetRemaining(), Units::ExToIn(100.0));

		// Market trade 1 and a bit orders
		marketOrderContainer = { { 6, Units::ExToIn(150.0) }, 0 };
		market->NewProcess<OtherSide>(marketOrderContainer, &marketWallets);
		limitOrders = Flatten(limitOrderMap);
		ASSERT_EQ(limitOrders.size(), 2u);
		ASSERT_DOUBLE_EQ(limitOrders.front().GetRemaining(), Units::ExToIn(50.0));

		// Market trade too much (should throw)
		marketOrderContainer = { { 6, Units::ExToIn(151.0) }, 0 };
		ASSERT_THROW(market->NewProcess<OtherSide>(marketOrderContainer, &marketWallets), Error);
		limitOrders = Flatten(limitOrderMap);
		ASSERT_EQ(limitOrders.size(), 2u); // Confirm the orders haven't been processed

		// Market trade all orders (the rest)
		marketOrderContainer = { { 6, Units::ExToIn(150.0) }, 0 };
		market->NewProcess<OtherSide>(marketOrderContainer, &marketWallets);
		limitOrders = Flatten(limitOrderMap);
		ASSERT_EQ(limitOrders.size(), 0u);
	}
};

TEST_F(OnlyLimitOrdersMarketTrade, sell) {
	setup<OrderAction::Sell>({ 0.4, 0.3, 0.2, 0.1 });
	check<OrderAction::Buy>(market->GetSellLimitOrderMap());
}

TEST_F(OnlyLimitOrdersMarketTrade, buy) {
	setup<OrderAction::Buy>({ 0.1, 0.2, 0.3, 0.4 });
	check<OrderAction::Sell>(market->GetBuyLimitOrderMap());
}
