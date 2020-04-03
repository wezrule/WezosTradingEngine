#include <TradingEngine/Orders/MarketOrder.h>
#include <TradingEngine/Orders/StopLimitOrder.h>
#include <TradingEngine/Units.h>
#include <gtest/gtest.h>

TEST(TestOrder, construction) {
	// Valid construction
	ASSERT_NO_THROW(LimitOrder(1, Units::ExToIn(100.0), Units::ExToIn(0.1)));
	ASSERT_NO_THROW(MarketOrder(1, Units::ExToIn(100.0)));
	ASSERT_NO_THROW(StopLimitOrder(1, Units::ExToIn(100.0), 0, Units::ExToIn(0.5)));
}
