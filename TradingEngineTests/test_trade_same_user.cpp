#include "StubListener.h"
#include "StubWallet.h"

#include <TradingEngine/Error.h>
#include <TradingEngine/Market.h>
#include <TradingEngine/Orders/MarketOrder.h>
#include <TradingEngine/Orders/OrderAction.h>
#include <TradingEngine/Orders/OrderContainer.h>
#include <TradingEngine/Orders/StopLimitOrder.h>
#include <TradingEngine/Units.h>
#include <TradingEngine/market_helper.h>
#include <cstdint>
#include <deque>
#include <gtest/gtest.h>
#include <memory>

MarketConfig createStubMarketConfig();

// Cannot buy/sell your own coins.
class TradeSameUserMarket : public ::testing::Test {
private:
	std::unique_ptr<Market> market;
	StubWallet stubWallet;
	MarketWallets marketWallets;

protected:
	template <OrderAction Side>
	void SetUp() {
		market = std::make_unique<Market>(std::make_unique<StubListener>(), CoinPair{ 4, 2 },
		createStubMarketConfig());
		marketWallets = { &stubWallet, &stubWallet };

		LimitOrder limitOrder{ 6, Units::ExToIn(100.0), 0 };
		OrderContainer limitOrderContainer{ limitOrder, Units::ExToIn(0.4) };
		market->NewProcess<Side>(limitOrderContainer, &marketWallets);
	}

	template <OrderAction OtherSide>
	void Check() {
		MarketOrder marketOrder{ 6, Units::ExToIn(25.2) };
		OrderContainer marketOrderContainer{ marketOrder, 0 };
		ASSERT_THROW((market->NewProcess<OtherSide>(marketOrderContainer, &marketWallets)), Error);
	}
};

TEST_F(TradeSameUserMarket, sell) {
	SetUp<OrderAction::Sell>();
	Check<OrderAction::Buy>();
}

TEST_F(TradeSameUserMarket, buy) {
	SetUp<OrderAction::Buy>();
	Check<OrderAction::Sell>();
}

class TradeSameUserLimit : public ::testing::Test {
protected:
	std::unique_ptr<Market> market;
	StubWallet stubWallet;
	MarketWallets marketWallets;

	template <OrderAction Side>
	void SetUp() {
		market = std::make_unique<Market>(std::make_unique<StubListener>(), CoinPair{ 4, 2 },
		createStubMarketConfig());
		marketWallets = { &stubWallet, &stubWallet };
		LimitOrder limitOrder{ 6, Units::ExToIn(100.0), 0 };
		market->NewProcess<Side>(OrderContainer{ limitOrder, Units::ExToIn(0.4) }, &marketWallets);
	}

	template <OrderAction OtherSide>
	void Check() {
		LimitOrder limitOrder{ 6, Units::ExToIn(25.2), 0 };
		OrderContainer limitOrderContainer{ limitOrder, Units::ExToIn(0.4) };
		ASSERT_THROW((market->NewProcess<OtherSide>(limitOrderContainer, &marketWallets)), Error);
	}
};

TEST_F(TradeSameUserLimit, sell) {
	SetUp<OrderAction::Sell>();
	Check<OrderAction::Buy>();
}

TEST_F(TradeSameUserLimit, buy) {
	SetUp<OrderAction::Buy>();
	Check<OrderAction::Sell>();
}

// Triggers off your own stop limit is allowed.
class TradeSameUserStopLimit : public ::testing::Test {
protected:
	std::unique_ptr<Market> market;
	StubWallet stubWallet;
	MarketWallets marketWallets;

	template <OrderAction Side, OrderAction OtherSide>
	void setUp() {
		market = std::make_unique<Market>(std::make_unique<StubListener>(), CoinPair{ 4, 2 },
		createStubMarketConfig());
		marketWallets = { &stubWallet, &stubWallet };
		LimitOrder limitOrder{ 6, Units::ExToIn(100.0), 0 };
		OrderContainer orderContainer{ limitOrder, Units::ExToIn(0.05) };
		market->NewProcess<Side>(orderContainer, &marketWallets);

		StopLimitOrder stopLimitOrder{ 7, Units::ExToIn(100.0), 0, Units::ExToIn(0.05) };
		OrderContainer stopLimitOrderContainer{ stopLimitOrder, Units::ExToIn(0.05) };
		market->NewProcess<OtherSide>(stopLimitOrderContainer, &marketWallets);
	}

	template <OrderAction OtherSide>
	void Process() {
		MarketOrder marketOrder{ 7, Units::ExToIn(50.0) };
		market->NewProcess<OtherSide>(OrderContainer{ marketOrder, 0 }, &marketWallets);
	}

	void Check(const std::deque<LimitOrder>& limitOrders,
	const std::deque<StopLimitOrder>& stopLimitOrders) {
		ASSERT_EQ(stopLimitOrders.size(), 0u);
		ASSERT_EQ(limitOrders.size(), 0u);
	}
};

TEST_F(TradeSameUserStopLimit, buy) {
	setUp<OrderAction::Buy, OrderAction::Sell>();
	Process<OrderAction::Sell>();
	Check(Flatten(market->GetBuyLimitOrderMap()), Flatten(market->GetSellStopLimitOrderMap()));
}

TEST_F(TradeSameUserStopLimit, sell) {
	setUp<OrderAction::Sell, OrderAction::Buy>();
	Process<OrderAction::Buy>();
	Check(Flatten(market->GetSellLimitOrderMap()), Flatten(market->GetBuyStopLimitOrderMap()));
}

// You should not be able to place a limit/stop-limit order where the price is such that,
// someone else in the future could trigger your own existing limit order.
class TradeSameUserLimitStopLimit : public ::testing::Test {
protected:
	std::unique_ptr<Market> market;
	StubWallet stubWallet;
	MarketWallets marketWallets;

	template <OrderAction Side>
	void SetUp(int64_t price) {
		market = std::make_unique<Market>(std::make_unique<StubListener>(), CoinPair{ 4, 2 },
		createStubMarketConfig());
		marketWallets = { &stubWallet, &stubWallet };
		LimitOrder limitOrder{ 7, Units::ExToIn(100.0), 0 };
		OrderContainer limitOrderContainer{ limitOrder, Units::ExToIn(0.05) };
		market->NewProcess<Side>(limitOrderContainer, &marketWallets);

		limitOrder = { 6, Units::ExToIn(100.0), 0 };
		market->NewProcess<Side>(OrderContainer{ limitOrder, price }, &marketWallets);
	}

	template <OrderAction OtherSide>
	void Process(int64_t invalidPrice, int64_t validPrice) {
		// The trigger price is the same as the user's existing, so all should fail,
		// as currently limit price can only be the same or less/greater depending on side.
		StopLimitOrder stopLimitOrder{ 7, Units::ExToIn(100.0), 0, Units::ExToIn(0.05) };
		OrderContainer stopLimitOrderContainer{ stopLimitOrder, Units::ExToIn(0.05) };
		ASSERT_THROW(market->NewProcess<OtherSide>(stopLimitOrderContainer, &marketWallets), Error);

		stopLimitOrderContainer = { stopLimitOrder, validPrice };
		ASSERT_THROW(market->NewProcess<OtherSide>(stopLimitOrderContainer, &marketWallets), Error);

		stopLimitOrderContainer = { stopLimitOrder, invalidPrice };
		ASSERT_THROW(market->NewProcess<OtherSide>(stopLimitOrderContainer, &marketWallets), Error);

		// Now try it where the trigger price is valid
		stopLimitOrder = { 7, Units::ExToIn(100.0), 0, validPrice };
		stopLimitOrderContainer = { stopLimitOrder, validPrice };
		ASSERT_NO_THROW(market->NewProcess<OtherSide>(stopLimitOrderContainer, &marketWallets));

		// As well as invalid
		stopLimitOrder = { 7, Units::ExToIn(100.0), 0, invalidPrice };
		stopLimitOrderContainer = { stopLimitOrder, invalidPrice };
		ASSERT_THROW(market->NewProcess<OtherSide>(stopLimitOrderContainer, &marketWallets), Error);
	}
};

TEST_F(TradeSameUserLimitStopLimit, buy) {
	SetUp<OrderAction::Buy>(Units::ExToIn(0.06));
	Process<OrderAction::Sell>(Units::ExToIn(0.06), Units::ExToIn(0.04));
}

TEST_F(TradeSameUserLimitStopLimit, sell) {
	SetUp<OrderAction::Sell>(Units::ExToIn(0.04));
	Process<OrderAction::Buy>(Units::ExToIn(0.04), Units::ExToIn(0.06));
}

// You should not be able to place a limit/stop-limit order where the price is such that,
// someone else in the future could trigger your own existing stop-limit order.
class TradeSameUserStopLimitLimit : public ::testing::Test {
protected:
	std::unique_ptr<Market> market;
	StubWallet stubWallet;
	MarketWallets marketWallets;

	template <OrderAction Side, OrderAction OtherSide>
	void setUp() {
		market = std::make_unique<Market>(std::make_unique<StubListener>(), CoinPair{ 4, 2 },
		createStubMarketConfig());
		marketWallets = { &stubWallet, &stubWallet };

		LimitOrder limitOrder{ 6, Units::ExToIn(100.0), 0 };
		OrderContainer limitOrderContainer{ limitOrder, Units::ExToIn(0.05) };
		market->NewProcess<Side>(limitOrderContainer, &marketWallets);

		StopLimitOrder stopLimitOrder{ 7, Units::ExToIn(100.0), 0, Units::ExToIn(0.05) };
		OrderContainer stopLimitOrderContainer{ stopLimitOrder, Units::ExToIn(0.05) };
		market->NewProcess<OtherSide>(stopLimitOrderContainer, &marketWallets);
	}

	template <OrderAction Side>
	void Process(double invalidPrice, double validPrice) {
		// Same price
		LimitOrder limitOrder{ 7, Units::ExToIn(100.0), 0 };
		OrderContainer limitOrderContainer{ limitOrder, Units::ExToIn(0.05) };
		ASSERT_THROW(market->NewProcess<Side>(limitOrderContainer, &marketWallets), Error);

		// Valid price
		limitOrderContainer = { limitOrder, Units::ExToIn(validPrice) };
		ASSERT_NO_THROW(market->NewProcess<Side>(limitOrderContainer, &marketWallets));

		// Invalid price
		limitOrderContainer = { limitOrder, Units::ExToIn(invalidPrice) };
		ASSERT_THROW(market->NewProcess<Side>(limitOrderContainer, &marketWallets), Error);
	}
};

TEST_F(TradeSameUserStopLimitLimit, buy) {
	setUp<OrderAction::Buy, OrderAction::Sell>();
	Process<OrderAction::Buy>(0.06, 0.04);
}

TEST_F(TradeSameUserStopLimitLimit, sell) {
	setUp<OrderAction::Sell, OrderAction::Buy>();
	Process<OrderAction::Sell>(0.04, 0.06);
}
