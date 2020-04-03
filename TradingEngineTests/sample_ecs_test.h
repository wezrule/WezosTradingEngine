#pragma once

#include "StubListener.h"
#include "StubWallet.h"

#include <TradingEngine/Market.h>
#include <TradingEngine/Orders/OrderAction.h>
#include <TradingEngine/Orders/OrderContainer.h>
#include <TradingEngine/Orders/StopLimitOrder.h>
#include <TradingEngine/Units.h>
#include <TradingEngine/Wallet.h>
#include <TradingEngine/market_helper.h>
#include <deque>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

MarketConfig createStubMarketConfig();

class SampleECSTest : public ::testing::Test {
protected:
	std::unique_ptr<Market> market;
	Wallet coinWallet;
	Wallet baseWallet;
	MarketWallets marketWallets;

	SampleECSTest() {
		{
			Address add{ 6 };
			add.SetTotalBalance(Units::ExToIn(1000.0));
			Address add1{ 8 };
			add1.SetTotalBalance(Units::ExToIn(1000.0));
			coinWallet.AddAddress(add);
			coinWallet.AddAddress(add1);

			baseWallet.AddAddress(add);
			baseWallet.AddAddress(add1);
		}

		marketWallets = { &coinWallet, &baseWallet };
		market = std::make_unique<Market>(std::make_unique<StubListener>(), CoinPair{ 4, 2 }, createStubMarketConfig());
	}

	template <OrderAction Side, OrderAction OtherSide>
	void SetUp() {
		// Limits
		auto price = Units::ExToIn(0.3);
		LimitOrder limitOrder{ 6, Units::ExToIn(100.0), 0 };
		market->NewProcess<Side>(OrderContainer{ limitOrder, price }, &marketWallets);
		market->NewProcess<Side>(OrderContainer{ limitOrder, price }, &marketWallets);

		limitOrder = { 6, Units::ExToIn(80.0), 0 };
		market->NewProcess<Side>(OrderContainer{ limitOrder, price }, &marketWallets);
		limitOrder = { 6, Units::ExToIn(120.0), 0 };
		market->NewProcess<Side>(OrderContainer{ limitOrder, price }, &marketWallets);

		// Stop limit
		StopLimitOrder stopLimitOrder{ 8, Units::ExToIn(100.0), 0, price };
		market->NewProcess<OtherSide>(OrderContainer{ stopLimitOrder, price }, &marketWallets);
		market->NewProcess<OtherSide>(OrderContainer{ stopLimitOrder, price }, &marketWallets);
		stopLimitOrder = { 8, Units::ExToIn(80.0), 0, price };
		market->NewProcess<OtherSide>(OrderContainer{ stopLimitOrder, price }, &marketWallets);

		stopLimitOrder = { 8, Units::ExToIn(120.0), 0, price };
		market->NewProcess<OtherSide>(OrderContainer{ stopLimitOrder, price }, &marketWallets);
	}

	void CheckSortedOrder(const std::deque<LimitOrder>& limitOrders,
	const std::deque<StopLimitOrder>& stopLimitOrders,
	const std::vector<int>& limitIds,
	const std::vector<int>& stopLimitIds) {
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
