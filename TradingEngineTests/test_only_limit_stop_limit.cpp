#include "StubListener.h"
#include "StubWallet.h"

#include <TradingEngine/Market.h>
#include <TradingEngine/Orders/MarketOrder.h>
#include <TradingEngine/Orders/OrderAction.h>
#include <TradingEngine/Orders/OrderContainer.h>
#include <TradingEngine/Orders/StopLimitOrder.h>
#include <TradingEngine/Units.h>
#include <TradingEngine/market_helper.h>
#include <chrono>
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <vector>

MarketConfig createStubMarketConfig();

class OnlyLimitAndStopLimit : public ::testing::Test {
protected:
	std::unique_ptr<Market> market;
	StubWallet stubWallet;
	MarketWallets marketWallets;

	template <OrderAction Side, OrderAction OtherSide>
	void SetUp(const std::vector<double>& limitRates, const std::vector<double>& stopRates,
	const std::vector<double>& stopLimitRates) {
		market = std::make_unique<Market>(std::make_unique<StubListener>(), CoinPair{ 4, 2 },
		createStubMarketConfig());

		marketWallets = { &stubWallet, &stubWallet };

		// Populate with initial orders.
		LimitOrder limitOrder{ 6, Units::ExToIn(15.0), 0 };
		OrderContainer limitOrderContainer{ limitOrder, Units::ExToIn(limitRates[0]) };
		market->NewProcess<Side>(limitOrderContainer, &marketWallets);

		limitOrder = { 6, Units::ExToIn(10.0), 0 };
		limitOrderContainer = { limitOrder, Units::ExToIn(limitRates[1]) };
		market->NewProcess<Side>(limitOrderContainer, &marketWallets);

		StopLimitOrder stopLimitOrder{ 7, Units::ExToIn(11.0), 0, Units::ExToIn(stopLimitRates[0]) };
		OrderContainer stopLimitOrderContainer{ stopLimitOrder, Units::ExToIn(stopRates[0]) };
		market->NewProcess<OtherSide>(stopLimitOrderContainer, &marketWallets);

		stopLimitOrder = { 7, Units::ExToIn(7.0), 0, Units::ExToIn(stopLimitRates[1]) };
		stopLimitOrderContainer = { stopLimitOrder, Units::ExToIn(stopRates[1]) };
		market->NewProcess<OtherSide>(stopLimitOrderContainer, &marketWallets);

		stopLimitOrder = { 7, Units::ExToIn(5.0), 0, Units::ExToIn(stopLimitRates[2]) };
		stopLimitOrderContainer = { stopLimitOrder, Units::ExToIn(stopRates[2]) };
		market->NewProcess<OtherSide>(stopLimitOrderContainer, &marketWallets);

		stopLimitOrder = { 7, Units::ExToIn(5.0), 0, Units::ExToIn(stopLimitRates[3]) };
		stopLimitOrderContainer = { stopLimitOrder, Units::ExToIn(stopRates[3]) };
		market->NewProcess<OtherSide>(stopLimitOrderContainer, &marketWallets);
	}

	template <OrderAction OtherSide, class Sort, class Sort1>
	void Check(const OrderMap<LimitOrder, Sort>& limitOrderMap,
	const OrderMap<StopLimitOrder, Sort>& otherStopLimitOrderMap,
	const OrderMap<LimitOrder, Sort1>& otherLimitOrderMap,
	const std::vector<double>& limitRates) {
		// Wait a bit just to make sure any new limit orders have current timestamp
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		// Make a trade, which fires off a couple of stop orders
		market->NewProcess<OtherSide>(OrderContainer<MarketOrder>{ { 7, Units::ExToIn(3.0) }, 0 },
		&marketWallets);
		auto limitOrders = Flatten(limitOrderMap);
		auto otherStopLimitOrders = Flatten(otherStopLimitOrderMap);
		auto otherLimitOrders = Flatten(otherLimitOrderMap);

		ASSERT_EQ(limitOrders.size(), 1u);
		ASSERT_EQ(otherLimitOrders.size(), 1u);
		ASSERT_EQ(otherStopLimitOrders.size(), 2u);

		ASSERT_EQ(limitOrders.back().GetId(), 2);
		ASSERT_EQ(limitOrders.back().GetRemaining(), Units::ExToIn(10.0));

		ASSERT_EQ(otherLimitOrders.back().GetId(), 4);
		ASSERT_EQ(otherLimitOrders.back().GetRemaining(), Units::ExToIn(6.0));

		// Make another trade which consumes the last limit order
		// (in addition from the next stop order which will be triggered)
		LimitOrder limitOrder{ 7, Units::ExToIn(5.0), 0 };
		OrderContainer limitOrderContainer{ limitOrder, Units::ExToIn(limitRates[0]) };
		market->NewProcess<OtherSide>(limitOrderContainer, &marketWallets);

		limitOrders = Flatten(limitOrderMap);
		otherStopLimitOrders = Flatten(otherStopLimitOrderMap);
		otherLimitOrders = Flatten(otherLimitOrderMap);

		ASSERT_EQ(limitOrders.size(), 0u);

		ASSERT_EQ(otherStopLimitOrders.size(), 1u);
		ASSERT_EQ(otherStopLimitOrders.back().GetId(), 6);
		ASSERT_EQ(otherStopLimitOrders.back().GetRemaining(), Units::ExToIn(5.0));

		ASSERT_EQ(otherLimitOrders.size(), 1u);
		ASSERT_EQ(otherLimitOrders.back().GetId(), 4);
		ASSERT_EQ(otherLimitOrders.back().GetRemaining(), Units::ExToIn(6.0));

		limitOrder = { 7, Units::ExToIn(1.0), 0 };
		limitOrderContainer = { limitOrder, Units::ExToIn(limitRates[1]) };

		// Make another which is the same as the stop rate, but shouldn't get called
		market->NewProcess<OtherSide>(limitOrderContainer, &marketWallets);

		limitOrders = Flatten(limitOrderMap);
		otherStopLimitOrders = Flatten(otherStopLimitOrderMap);
		otherLimitOrders = Flatten(otherLimitOrderMap);

		ASSERT_EQ(limitOrders.size(), 0u);

		ASSERT_EQ(otherStopLimitOrders.size(), 1u);
		ASSERT_EQ(otherStopLimitOrders.back().GetId(), 6);
		ASSERT_EQ(otherStopLimitOrders.back().GetRemaining(), Units::ExToIn(5.0));

		// TODO: These should be swapped
		ASSERT_EQ(otherLimitOrders.size(), 2u);
		ASSERT_EQ(otherLimitOrders.front().GetId(), 9);
		ASSERT_EQ(otherLimitOrders.front().GetRemaining(), Units::ExToIn(1.0));
		ASSERT_EQ(otherLimitOrders.back().GetId(), 4);
		ASSERT_EQ(otherLimitOrders.back().GetRemaining(), Units::ExToIn(6.0));
	}
};

TEST_F(OnlyLimitAndStopLimit, sell) {
	std::vector<double> stopLimitRates{ 0.2, 0.1, 0.2, 2.0 };
	SetUp<OrderAction::Sell, OrderAction::Buy>({ 0.1, 0.2 }, { 0.1, 0.1, 0.2, 1.0 },
	stopLimitRates);

	Check<OrderAction::Buy>(market->GetSellLimitOrderMap(), market->GetBuyStopLimitOrderMap(),
	market->GetBuyLimitOrderMap(), { 0.3, 2 });
}

TEST_F(OnlyLimitAndStopLimit, buy) {
	std::vector<double> stopLimitRates{ 0.05, 0.2, 0.1, 0.03 };
	SetUp<OrderAction::Buy, OrderAction::Sell>({ 0.2, 0.1 }, { 0.2, 0.2, 0.1, 0.04 },
	stopLimitRates);

	Check<OrderAction::Sell>(market->GetBuyLimitOrderMap(), market->GetSellStopLimitOrderMap(),
	market->GetSellLimitOrderMap(), { 0.1, 0.04 });
}
