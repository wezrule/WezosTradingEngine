#include "StubListener.h"
#include "StubWallet.h"

#include <TradingEngine/Market.h>
#include <TradingEngine/Orders/OrderAction.h>
#include <TradingEngine/Orders/OrderContainer.h>
#include <TradingEngine/Orders/StopLimitOrder.h>
#include <TradingEngine/Units.h>
#include <TradingEngine/market_helper.h>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

MarketConfig createStubMarketConfig();

class SortDifferentPrices : public ::testing::Test {
protected:
	std::unique_ptr<Market> market;
	StubWallet stubWallet;
	MarketWallets marketWallets;

	template <OrderAction Side, OrderAction OtherSide>
	void SetUp(std::vector<double> stopRatesIds) {
		market = std::make_unique<Market>(std::make_unique<StubListener>(), CoinPair{ 4, 2 },
		createStubMarketConfig());
		marketWallets = { &stubWallet, &stubWallet };

		LimitOrder limitOrder{ 6, Units::ExToIn(100.0), Units::ExToIn(0.1) };
		market->NewProcess<Side>(OrderContainer{ limitOrder, Units::ExToIn(0.5) }, &marketWallets);

		// Different price
		market->NewProcess<Side>(OrderContainer{ limitOrder, Units::ExToIn(0.2) }, &marketWallets);
		market->NewProcess<Side>(OrderContainer{ limitOrder, Units::ExToIn(0.4) }, &marketWallets);
		market->NewProcess<Side>(OrderContainer{ limitOrder, Units::ExToIn(0.3) }, &marketWallets);
		market->NewProcess<Side>(OrderContainer{ limitOrder, Units::ExToIn(0.1) }, &marketWallets);
		market->NewProcess<Side>(OrderContainer{ limitOrder, Units::ExToIn(0.6) }, &marketWallets);

		// Same rate (later one should be behind)
		market->NewProcess<Side>(OrderContainer{ limitOrder, Units::ExToIn(0.2) }, &marketWallets);

		// Stop limits
		StopLimitOrder stopLimitOrder{ 8, Units::ExToIn(100.0), 0, Units::ExToIn(stopRatesIds[0]) };
		OrderContainer stopLimitOrderContainer{ stopLimitOrder, Units::ExToIn(stopRatesIds[0]) };
		market->NewProcess<OtherSide>(stopLimitOrderContainer, &marketWallets);

		stopLimitOrder = { 8, Units::ExToIn(100.0), 0, Units::ExToIn(stopRatesIds[1]) };
		stopLimitOrderContainer = { stopLimitOrder, Units::ExToIn(stopRatesIds[1]) };
		market->NewProcess<OtherSide>(stopLimitOrderContainer, &marketWallets);

		stopLimitOrder = { 8, Units::ExToIn(100.0), 0, Units::ExToIn(stopRatesIds[2]) };
		stopLimitOrderContainer = { stopLimitOrder, Units::ExToIn(stopRatesIds[2]) };
		market->NewProcess<OtherSide>(stopLimitOrderContainer, &marketWallets);
	}

	template <class Sort>
	void checkSortedOrder(const OrderMap<LimitOrder, Sort>& limitOrderMap,
	const OrderMap<StopLimitOrder, Sort>& stopLimitOrderMap,
	const std::vector<int64_t>& limitIds,
	const std::vector<int64_t>& stopLimitIds) {
		auto limitOrders = Flatten(limitOrderMap);
		auto stopLimitOrders = Flatten(stopLimitOrderMap);

		ASSERT_EQ(limitOrders.size(), limitOrders.size());
		for (auto i = 0u; i < limitIds.size(); ++i) {
			ASSERT_EQ(limitOrders[i].GetId(), limitIds[i]);
		}

		ASSERT_EQ(stopLimitOrders.size(), stopLimitIds.size());
		for (auto i = 0u; i < stopLimitIds.size(); ++i) {
			ASSERT_EQ(stopLimitOrders[i].GetId(), stopLimitIds[i]);
		}
	}
};

TEST_F(SortDifferentPrices, sellOnly) {
	SetUp<OrderAction::Sell, OrderAction::Buy>({ 0.85, 0.75, 0.95 });

	std::vector<int64_t> limitIds{ 5, 2, 7, 4, 3, 1, 6 };
	std::vector<int64_t> stopLimitIds{ 9, 8, 10 };
	checkSortedOrder(market->GetSellLimitOrderMap(), market->GetBuyStopLimitOrderMap(), limitIds,
	stopLimitIds);
}

TEST_F(SortDifferentPrices, buyOnly) {
	SetUp<OrderAction::Buy, OrderAction::Sell>({ 0.35, 0.25, 0.45 });

	std::vector<int64_t> limitIds{ 6, 1, 3, 4, 2, 7, 5 };
	std::vector<int64_t> stopLimitIds{ 10, 8, 9 };
	checkSortedOrder(market->GetBuyLimitOrderMap(), market->GetSellStopLimitOrderMap(), limitIds,
	stopLimitIds);
}
