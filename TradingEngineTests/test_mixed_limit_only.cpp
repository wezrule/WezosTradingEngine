#include "StubListener.h"
#include "StubWallet.h"

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

class MixedLimitOnly : public ::testing::Test {
protected:
	std::unique_ptr<Market> market;
	StubWallet stubWallet;
	MarketWallets marketWallets;

	void SetUp() {
		market = std::make_unique<Market>(std::make_unique<StubListener>(), CoinPair{ 4, 2 },
		createStubMarketConfig());

		marketWallets = { &stubWallet, &stubWallet };

		// Populate with initial orders
		LimitOrder limitOrder{ 6, Units::ExToIn(10.0), 0 };
		OrderContainer limitOrderContainer{ limitOrder, Units::ExToIn(0.3) };
		market->NewProcess<OrderAction::Sell>(limitOrderContainer, &marketWallets);

		limitOrderContainer = { limitOrder, Units::ExToIn(0.2) };
		market->NewProcess<OrderAction::Sell>(limitOrderContainer, &marketWallets);

		limitOrderContainer = { limitOrder, Units::ExToIn(0.1) };
		market->NewProcess<OrderAction::Sell>(limitOrderContainer, &marketWallets);

		limitOrderContainer = { limitOrder, Units::ExToIn(0.05) };
		market->NewProcess<OrderAction::Buy>(limitOrderContainer, &marketWallets);

		limitOrderContainer = { limitOrder, Units::ExToIn(0.04) };
		market->NewProcess<OrderAction::Buy>(limitOrderContainer, &marketWallets);

		limitOrderContainer = { limitOrder, Units::ExToIn(0.03) };
		market->NewProcess<OrderAction::Buy>(limitOrderContainer, &marketWallets);
	}

	template <OrderAction Side, class Sort, class Sort1>
	void Check(const OrderMap<LimitOrder, Sort>& limitOrderMap,
	const OrderMap<LimitOrder, Sort1>& otherLimitOrderMap, const std::vector<int>& ids,
	const std::vector<double>& rates) {
		{
			// Partial (eat into the first other order type)
			MarketOrder marketOrder{ 7, Units::ExToIn(5.0) };
			market->NewProcess<Side>(OrderContainer{ marketOrder, 0 }, &marketWallets);
			auto limitOrders = Flatten(limitOrderMap);
			auto otherLimitOrders = Flatten(otherLimitOrderMap);

			// Confirm orders are unaffected
			ASSERT_EQ(limitOrders.front().GetId(), ids[3]);
			ASSERT_DOUBLE_EQ(limitOrders.front().GetRemaining(), Units::ExToIn(10.0));

			ASSERT_EQ(limitOrders[1].GetId(), ids[4]);
			ASSERT_DOUBLE_EQ(limitOrders[1].GetRemaining(), Units::ExToIn(10.0));

			ASSERT_EQ(limitOrders.back().GetId(), ids[5]);
			ASSERT_DOUBLE_EQ(limitOrders.back().GetRemaining(), Units::ExToIn(10.0));

			// Confirm other orders are unaffected, except the last one which is eaten into
			ASSERT_EQ(otherLimitOrders.front().GetId(), ids[2]);
			ASSERT_DOUBLE_EQ(otherLimitOrders.front().GetRemaining(), Units::ExToIn(5.0));

			ASSERT_EQ(otherLimitOrders[1].GetId(), ids[1]);
			ASSERT_DOUBLE_EQ(otherLimitOrders[1].GetRemaining(), Units::ExToIn(10.0));

			ASSERT_EQ(otherLimitOrders.back().GetId(), ids[0]);
			ASSERT_DOUBLE_EQ(otherLimitOrders.back().GetRemaining(), Units::ExToIn(10.0));
		}

		{
			// Now finish that other order
			LimitOrder limitOrder{ 7, Units::ExToIn(5.0), 0 };
			OrderContainer limitOrderContainer{ limitOrder, Units::ExToIn(rates[0]) };
			market->NewProcess<Side>(limitOrderContainer, &marketWallets);
			auto limitOrders = Flatten(limitOrderMap);
			auto otherLimitOrders = Flatten(otherLimitOrderMap);

			// Confirm orders are unaffected (duplicate)
			ASSERT_EQ(limitOrders.front().GetId(), ids[3]);
			ASSERT_DOUBLE_EQ(limitOrders.front().GetRemaining(), Units::ExToIn(10.0));

			ASSERT_EQ(limitOrders[1].GetId(), ids[4]);
			ASSERT_DOUBLE_EQ(limitOrders[1].GetRemaining(), Units::ExToIn(10.0));

			ASSERT_EQ(limitOrders.back().GetId(), ids[5]);
			ASSERT_DOUBLE_EQ(limitOrders.back().GetRemaining(), Units::ExToIn(10.0));

			// Should only be 2 of the other order left
			ASSERT_EQ(otherLimitOrders.size(), 2u);
			ASSERT_EQ(otherLimitOrders.front().GetId(), ids[1]);
			ASSERT_DOUBLE_EQ(otherLimitOrders.front().GetRemaining(), Units::ExToIn(10.0));

			ASSERT_EQ(otherLimitOrders.back().GetId(), ids[0]);
			ASSERT_DOUBLE_EQ(otherLimitOrders.back().GetRemaining(), Units::ExToIn(10.0));
		}

		{
			// === Trade between other orders, so a new limit order is created for the order limit price ===
			LimitOrder limitOrder{ 7, Units::ExToIn(12.0), 0 };
			OrderContainer limitOrderContainer{ limitOrder, Units::ExToIn(rates[1]) };
			market->NewProcess<Side>(limitOrderContainer, &marketWallets);
			auto limitOrders = Flatten(limitOrderMap);
			auto otherLimitOrders = Flatten(otherLimitOrderMap);

			// Will consume that other order, leaving 1 remaining
			ASSERT_EQ(otherLimitOrders.size(), 1u);
			ASSERT_EQ(otherLimitOrders.back().GetId(), ids[0]);
			ASSERT_DOUBLE_EQ(otherLimitOrders.back().GetRemaining(), Units::ExToIn(10.0));

			// Should be the same orders, but also an additional one at the back
			ASSERT_EQ(limitOrders.front().GetId(), 9);
			ASSERT_DOUBLE_EQ(limitOrders.front().GetRemaining(), Units::ExToIn(2.0));

			ASSERT_EQ(limitOrders[1].GetId(), ids[3]);
			ASSERT_DOUBLE_EQ(limitOrders[1].GetRemaining(), Units::ExToIn(10.0));

			ASSERT_EQ(limitOrders[2].GetId(), ids[4]);
			ASSERT_DOUBLE_EQ(limitOrders[2].GetRemaining(), Units::ExToIn(10.0));

			ASSERT_EQ(limitOrders.back().GetId(), ids[5]);
			ASSERT_DOUBLE_EQ(limitOrders.back().GetRemaining(), Units::ExToIn(10.0));
		}

		{
			// === Trade behind other order so all other orders are consumed, and a new limit order is created ===
			LimitOrder limitOrder{ 7, Units::ExToIn(13.0), 0 };
			OrderContainer limitOrderContainer{ limitOrder, Units::ExToIn(rates[2]) };
			market->NewProcess<Side>(limitOrderContainer, &marketWallets);
			auto limitOrders = Flatten(limitOrderMap);
			auto otherLimitOrders = Flatten(otherLimitOrderMap);

			// Will consume that other order, leaving 0 remaining
			ASSERT_EQ(otherLimitOrders.size(), 0u);

			// Should be the same orders, but also an additional one at the back
			ASSERT_EQ(limitOrders.front().GetId(), 10);
			ASSERT_DOUBLE_EQ(limitOrders.front().GetRemaining(), Units::ExToIn(3.0));

			ASSERT_EQ(limitOrders[1].GetId(), 9);
			ASSERT_DOUBLE_EQ(limitOrders[1].GetRemaining(), Units::ExToIn(2.0));

			ASSERT_EQ(limitOrders[2].GetId(), ids[3]);
			ASSERT_DOUBLE_EQ(limitOrders[2].GetRemaining(), Units::ExToIn(10.0));

			ASSERT_EQ(limitOrders[3].GetId(), ids[4]);
			ASSERT_DOUBLE_EQ(limitOrders[3].GetRemaining(), Units::ExToIn(10.0));

			ASSERT_EQ(limitOrders.back().GetId(), ids[5]);
			ASSERT_DOUBLE_EQ(limitOrders.back().GetRemaining(), Units::ExToIn(10.0));
		}
	}
};

TEST_F(MixedLimitOnly, sell) {
	SetUp();
	Check<OrderAction::Sell>(market->GetSellLimitOrderMap(), market->GetBuyLimitOrderMap(),
	{ 6, 5, 4, 3, 2, 1 }, { 0.045, 0.035, 0.025 });
}

TEST_F(MixedLimitOnly, buy) {
	SetUp();
	Check<OrderAction::Buy>(market->GetBuyLimitOrderMap(), market->GetSellLimitOrderMap(),
	{ 1, 2, 3, 4, 5, 6 }, { 0.15, 0.25, 0.35 });
}
