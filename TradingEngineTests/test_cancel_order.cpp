#include "sample_ecs_test.h"

#include <TradingEngine/Error.h>
#include <TradingEngine/Market.h>
#include <TradingEngine/Orders/OrderAction.h>
#include <TradingEngine/Orders/StopLimitOrder.h>
#include <TradingEngine/Units.h>
#include <TradingEngine/market_helper.h>
#include <deque>
#include <gtest/gtest.h>

class CancelOrder : public SampleECSTest {
protected:
	template <OrderAction Side, OrderAction OtherSide, class Comp>
	void Check(const OrderMap<LimitOrder, Comp>& limitOrderMap,
	const OrderMap<StopLimitOrder, Comp>& stopLimitOrderMap) {
		market->CancelOrder<Side, LimitOrder>(3, Units::ExToIn(0.3), &marketWallets);
		auto limitOrders = Flatten(limitOrderMap);
		auto stopLimitOrders = Flatten(stopLimitOrderMap);

		CheckSortedOrder(limitOrders, stopLimitOrders, { 1, 2, 4 }, { 5, 6, 7, 8 });

		// Trying to remove the same again
		ASSERT_THROW((market->CancelOrder<Side, LimitOrder>(3, Units::ExToIn(0.3), &marketWallets)),
		Error);

		// Trying to remove one with the wrong price (for the id)
		ASSERT_THROW((market->CancelOrder<Side, LimitOrder>(9, Units::ExToIn(0.4), &marketWallets)),
		Error);

		market->CancelOrder<Side, LimitOrder>(1, Units::ExToIn(0.3), &marketWallets);
		market->CancelOrder<OtherSide, StopLimitOrder>(7, Units::ExToIn(0.3), &marketWallets);
		market->CancelOrder<OtherSide, StopLimitOrder>(6, Units::ExToIn(0.3), &marketWallets);
		limitOrders = Flatten(limitOrderMap);
		stopLimitOrders = Flatten(stopLimitOrderMap);

		CheckSortedOrder(limitOrders, stopLimitOrders, { 2, 4 }, { 5, 8 });
	}
};

template <OrderAction Side, OrderAction OtherSide>
void CheckUserOrderCache(Market* market) {
	const auto& priceOrderIds = market->GetUserOrderCache<Side, LimitOrder>(6);
	ASSERT_EQ(priceOrderIds.size(), 2u);
	ASSERT_EQ(priceOrderIds[0].orderId, 2);
	ASSERT_EQ(priceOrderIds[1].orderId, 4);

	const auto& priceOrderIds1 = market->GetUserOrderCache<OtherSide, StopLimitOrder>(8);
	ASSERT_EQ(priceOrderIds1.size(), 2u);
	ASSERT_EQ(priceOrderIds1[0].orderId, 5);
	ASSERT_EQ(priceOrderIds1[1].orderId, 8);
}

TEST_F(CancelOrder, sell) {
	ASSERT_THROW(
	(market->CancelOrder<OrderAction::Sell, LimitOrder>(3, Units::ExToIn(0.3), &marketWallets)),
	Error); // There are no sell orders
	SetUp<OrderAction::Sell, OrderAction::Buy>();
	Check<OrderAction::Sell, OrderAction::Buy>(market->GetSellLimitOrderMap(),
	market->GetBuyStopLimitOrderMap());

	// Check wallets
	ASSERT_EQ(coinWallet.GetAddress(6)->GetInOrder(), Units::ExToIn(220.0));
	ASSERT_EQ(baseWallet.GetAddress(8)->GetInOrder(), Units::ExToIn(220.0));

	CheckUserOrderCache<OrderAction::Sell, OrderAction::Buy>(market.get());
}

TEST_F(CancelOrder, buy) {
	ASSERT_THROW(
	(market->CancelOrder<OrderAction::Buy, LimitOrder>(3, Units::ExToIn(0.3), &marketWallets)),
	Error); // There are no buy orders

	SetUp<OrderAction::Buy, OrderAction::Sell>();
	Check<OrderAction::Buy, OrderAction::Sell>(market->GetBuyLimitOrderMap(),
	market->GetSellStopLimitOrderMap());

	// Check wallets
	ASSERT_EQ(coinWallet.GetAddress(8)->GetInOrder(), Units::ExToIn(220.0));
	ASSERT_EQ(baseWallet.GetAddress(6)->GetInOrder(), Units::ExToIn(220.0));

	CheckUserOrderCache<OrderAction::Buy, OrderAction::Sell>(market.get());
}

TEST_F(CancelOrder, AllOrders) {
	SetUp<OrderAction::Buy, OrderAction::Sell>();
	market->CancelAll();
	ASSERT_EQ((market->GetUserOrderMap().size()), 0u);
	ASSERT_EQ(market->GetBuyLimitOrderMap().size(), 0u);
	ASSERT_EQ(market->GetSellStopLimitOrderMap().size(), 0u);
}

TEST_F(CancelOrder, AllOrdersSingleMarketUser) {
	SetUp<OrderAction::Buy, OrderAction::Sell>();

	auto userId = 6;
	market->CancelAll(userId);
	ASSERT_EQ((market->GetUserOrderCache<OrderAction::Buy, LimitOrder>(6).size()), 0u);
	ASSERT_EQ(market->GetBuyLimitOrderMap().size(), 0u);
	ASSERT_EQ(Flatten(market->GetSellStopLimitOrderMap()).size(), 4u);
}
