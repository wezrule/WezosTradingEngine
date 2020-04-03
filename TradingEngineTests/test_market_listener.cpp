#include "StubWallet.h"
#include "mock_listener.h"

#include <TradingEngine/Fee.h>
#include <TradingEngine/Listener/ListenerOrder.h>
#include <TradingEngine/Market.h>
#include <TradingEngine/Orders/MarketOrder.h>
#include <TradingEngine/Orders/OrderAction.h>
#include <TradingEngine/Orders/OrderContainer.h>
#include <TradingEngine/Orders/StopLimitOrder.h>
#include <TradingEngine/Units.h>
#include <TradingEngine/market_helper.h>
#include <algorithm>
#include <cstdint>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

MarketConfig createStubMarketConfig();

void AddSellStopLimitOrder(Market& market, MarketWallets& marketWallets,
MockListener& mockListener, int64_t stopPrice, int64_t id) {
	StopLimitOrder sellStopLimitOrder{ 6, Units::ExToIn(50.0), 0, stopPrice };
	auto expectedSellStopLimitOrder = sellStopLimitOrder;
	expectedSellStopLimitOrder.SetId(id);
	auto price = Units::ExToIn(0.2);

	OrderContainer expectedOrderContainer{ expectedSellStopLimitOrder, price };
	auto listenerOrder = ConvertToListenerOrder(expectedOrderContainer);
	EXPECT_CALL(mockListener, NewOpenOrder(listenerOrder, OrderAction::Sell)).Times(1);

	OrderContainer orderContainer{ sellStopLimitOrder, price };
	market.NewProcess<OrderAction::Sell>(orderContainer, &marketWallets);
}

TEST(TestMarketListener, mockListener) {
	auto listener = std::make_unique<MockListener>();
	StubWallet stubWallet;

	MarketWallets marketWallets{ &stubWallet, &stubWallet };

	auto marketConfig = createStubMarketConfig();
	auto fee = marketConfig.feeDivision;

	Market market(std::move(listener), { 4, 2 }, marketConfig);

	auto& mockListener = (MockListener&)market.GetListener();

	// Insert a limit sell order
	{
		LimitOrder sellLimitOrder{ 6, Units::ExToIn(100.0), 0 };
		auto expectedSellLimitOrder = sellLimitOrder;
		expectedSellLimitOrder.SetId(1);
		auto price = Units::ExToIn(0.1);
		OrderContainer expectedOrderContainer{ expectedSellLimitOrder, price };
		auto listenerOrder = ConvertToListenerOrder(expectedOrderContainer);
		EXPECT_CALL(mockListener, NewOpenOrder(listenerOrder, OrderAction::Sell)).Times(1);
		OrderContainer orderContainer{ sellLimitOrder, price };
		market.NewProcess<OrderAction::Sell>(orderContainer, &marketWallets);
	}

	// Add a buy limit order which consumes the sell order, and inserts the remainder as a buy limit order
	{
		LimitOrder buyLimitOrder{ 7, Units::ExToIn(150.0), 0 };
		auto expectedBuyLimitOrder = buyLimitOrder;
		expectedBuyLimitOrder.SetId(2);
		expectedBuyLimitOrder.AddToFill(Units::ExToIn(100.0));

		auto buyFee = Units::ExToIn(100.0) / fee;
		auto sellFee = Units::ScaleDown(Units::ExToIn(100.0) * Units::ExToIn(0.1)) / fee;
		Fee fees{ buyFee, sellFee };
		auto price = Units::ExToIn(0.2);

		OrderContainer expectedOrderContainer{ expectedBuyLimitOrder, price };
		auto listenerOrder = ConvertToListenerOrder(expectedOrderContainer);
		EXPECT_CALL(mockListener, NewOpenOrder(listenerOrder, OrderAction::Buy)).Times(1);
		EXPECT_CALL(mockListener, NewTrade(1, 2, 1, Units::ExToIn(100.0), Units::ExToIn(0.1), fees))
		.Times(1);
		EXPECT_CALL(mockListener, OrderFilled(1)).Times(1);

		OrderContainer orderContainer{ buyLimitOrder, price };
		market.NewProcess<OrderAction::Buy>(orderContainer, &marketWallets);
	}

	// A market sell order to consume a bit of the remaining buy order
	{
		MarketOrder marketSellOrder{ 6, Units::ExToIn(25.0) }; // Id 3
		auto expectedMarketSellOrder = marketSellOrder;
		expectedMarketSellOrder.SetId(3);
		expectedMarketSellOrder.AddToFill(Units::ExToIn(25.0));

		auto buyFee = Units::ExToIn(25.0) / fee;
		auto sellFee = Units::ScaleDown(Units::ExToIn(25.0) * Units::ExToIn(0.2)) / fee;
		Fee fees{ buyFee, sellFee };

		OrderContainer expectedOrderContainer{ expectedMarketSellOrder, 0 };
		auto listenerOrder = ConvertToListenerOrder(expectedOrderContainer);
		EXPECT_CALL(mockListener, NewFilledOrder(listenerOrder, OrderAction::Sell)).Times(1);
		EXPECT_CALL(mockListener, NewTrade(2, 2, 3, Units::ExToIn(25.0), Units::ExToIn(0.2), fees)).Times(1);
		EXPECT_CALL(mockListener, PartialFill(2, Units::ExToIn(25.0))).Times(1);

		OrderContainer orderContainer{ marketSellOrder, 0 };
		market.NewProcess<OrderAction::Sell>(orderContainer, &marketWallets);
	}

	// Insert a buy limit order (just gets inserted), at same rate.
	{
		LimitOrder buyLimitOrder{ 7, Units::ExToIn(100.0), 0 }; // Id 4
		auto expectedBuyLimitOrder = buyLimitOrder;
		expectedBuyLimitOrder.SetId(4);
		auto price = Units::ExToIn(0.2);

		OrderContainer expectedOrderContainer{ expectedBuyLimitOrder, price };
		auto listenerOrder = ConvertToListenerOrder(expectedOrderContainer);
		EXPECT_CALL(mockListener, NewOpenOrder(listenerOrder, OrderAction::Buy)).Times(1);

		OrderContainer orderContainer{ buyLimitOrder, price };
		market.NewProcess<OrderAction::Buy>(orderContainer, &marketWallets);
	}

	// Add 2 sell stop orders (just get inserted)
	AddSellStopLimitOrder(market, marketWallets, mockListener, Units::ExToIn(0.1), 5);
	AddSellStopLimitOrder(market, marketWallets, mockListener, Units::ExToIn(0.2), 6);

	// There is now 2 buy orders, 100, 25, and 2 sell stop limit. Trigger them both
	MarketOrder marketSellOrder{ 6, Units::ExToIn(55.0) }; // Id 7
	auto expectedMarketSellOrder = marketSellOrder;
	expectedMarketSellOrder.SetId(7);
	expectedMarketSellOrder.AddToFill(Units::ExToIn(55.0));

	auto buyFees = Units::ExToIn(25.0) / fee;
	auto sellFees = Units::ScaleDown(Units::ExToIn(25.0) * Units::ExToIn(0.2)) / fee;
	Fee fees{ buyFees, sellFees };

	EXPECT_CALL(mockListener, NewTrade(3, 2, 7, Units::ExToIn(25.0), Units::ExToIn(0.2), fees)).Times(1);
	EXPECT_CALL(mockListener, OrderFilled(2)).Times(1);

	buyFees = Units::ExToIn(30.0) / fee;
	sellFees = Units::ScaleDown(Units::ExToIn(30.0) * Units::ExToIn(0.2)) / fee;
	fees = { buyFees, sellFees };

	EXPECT_CALL(mockListener, NewTrade(4, 4, 7, Units::ExToIn(30.0), Units::ExToIn(0.2), fees)).Times(1);
	EXPECT_CALL(mockListener, PartialFill(4, Units::ExToIn(30.0))).Times(1);

	OrderContainer orderContainer{ expectedMarketSellOrder, 0 };
	auto listenerOrder = ConvertToListenerOrder(orderContainer);
	EXPECT_CALL(mockListener, NewFilledOrder(listenerOrder, OrderAction::Sell)).Times(1);

	// Stop orders get processed now (first one)
	EXPECT_CALL(mockListener, StopLimitTriggered(5, 4)).Times(1);

	buyFees = Units::ExToIn(50.0) / fee;
	sellFees = Units::ScaleDown(Units::ExToIn(50.0) * Units::ExToIn(0.2)) / fee;
	fees = { buyFees, sellFees };
	LimitOrder expectedLimitSellOrder{ 6, Units::ExToIn(50.0), Units::ExToIn(50.0) }; // Id 5
	expectedLimitSellOrder.SetId(5);
	EXPECT_CALL(mockListener, NewTrade(5, 4, 5, Units::ExToIn(50.0), Units::ExToIn(0.2), fees)).Times(1);
	EXPECT_CALL(mockListener, PartialFill(4, Units::ExToIn(50.0))).Times(1);

	OrderContainer limitOrderContainer = { expectedLimitSellOrder, Units::ExToIn(0.1) };
	listenerOrder = ConvertToListenerOrder(limitOrderContainer);
	EXPECT_CALL(mockListener, NewFilledOrder(listenerOrder, OrderAction::Sell)).Times(1);

	// Second stop order
	buyFees = Units::ExToIn(20.0) / fee;
	sellFees = Units::ScaleDown(Units::ExToIn(20.0) * Units::ExToIn(0.2)) / fee;
	fees = { buyFees, sellFees };
	EXPECT_CALL(mockListener, StopLimitTriggered(6, 4)).Times(1);
	EXPECT_CALL(mockListener, NewTrade(6, 4, 6, Units::ExToIn(20.0), Units::ExToIn(0.2), fees)).Times(1);
	EXPECT_CALL(mockListener, OrderFilled(4)).Times(1);

	LimitOrder expectedLimitSellOrderFromStop{ 6, Units::ExToIn(50.0), Units::ExToIn(20.0) }; // Id 6
	expectedLimitSellOrderFromStop.SetId(6);

	limitOrderContainer = { expectedLimitSellOrderFromStop, Units::ExToIn(0.2) };
	listenerOrder = ConvertToListenerOrder(limitOrderContainer);
	EXPECT_CALL(mockListener, NewOpenOrder(listenerOrder, OrderAction::Sell)).Times(1);

	orderContainer = { marketSellOrder, 0 };
	market.NewProcess<OrderAction::Sell>(orderContainer, &marketWallets);
}
