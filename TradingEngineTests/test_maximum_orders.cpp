#include "StubListener.h"
#include "StubWallet.h"

#include <TradingEngine/Error.h>
#include <TradingEngine/Market.h>
#include <TradingEngine/Orders/LimitOrder.h>
#include <TradingEngine/Orders/OrderAction.h>
#include <TradingEngine/Orders/OrderContainer.h>
#include <TradingEngine/Units.h>
#include <TradingEngine/market_helper.h>
#include <gtest/gtest.h>
#include <memory>

TEST(TestMaximum, MaxNumOpenOrders) {
	StubWallet stubWallet;

	MarketWallets marketWallets{ &stubWallet, &stubWallet };

	constexpr auto maxOpenOrders = 5;
	MarketConfig config{ 1000, maxOpenOrders };

	Market market(std::make_unique<StubListener>(), CoinPair{ 4, 2 }, config);

	LimitOrder limitOrder{ 7, Units::ExToIn(100.0), 0 };
	OrderContainer limitOrderContainer{ limitOrder, Units::ExToIn(0.3) };
	for (int i = 0; i < maxOpenOrders; ++i) {
		market.NewProcess<OrderAction::Sell>(limitOrderContainer, &marketWallets);
	}

	// Now try add another one, it should throw..
	ASSERT_THROW(market.NewProcess<OrderAction::Sell>(limitOrderContainer, &marketWallets),
	Error);
}
