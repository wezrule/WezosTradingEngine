#include "gtest/gtest.h"
#include <TradingEngine/MessageType.h>

TEST(TestError, messageTypeOrdering) {
	ASSERT_EQ(static_cast<int>(MessageType::MarketOrder), 0);
	ASSERT_EQ(static_cast<int>(MessageType::LimitOrder), 1);
	ASSERT_EQ(static_cast<int>(MessageType::StopLimitOrder), 2);
	ASSERT_EQ(static_cast<int>(MessageType::CancelOrder), 3);
	ASSERT_EQ(static_cast<int>(MessageType::CancelAllOrders), 4);
	ASSERT_EQ(static_cast<int>(MessageType::Deposit), 5);
	ASSERT_EQ(static_cast<int>(MessageType::Withdraw), 6);
	ASSERT_EQ(static_cast<int>(MessageType::NewCoin), 7);
	ASSERT_EQ(static_cast<int>(MessageType::NewMarket), 8);
	ASSERT_EQ(static_cast<int>(MessageType::SetFeePercentage), 9);
	ASSERT_EQ(static_cast<int>(MessageType::SetMaxNumLimitOpenOrders), 10);
	ASSERT_EQ(static_cast<int>(MessageType::SetMaxNumStopLimitOpenOrders), 11);
	ASSERT_EQ(static_cast<int>(MessageType::SetInOrder), 12);
	ASSERT_EQ(static_cast<int>(MessageType::GetAmount), 13);
	ASSERT_EQ(static_cast<int>(MessageType::GetAvailable), 14);
	ASSERT_EQ(static_cast<int>(MessageType::GetInOrder), 15);
	ASSERT_EQ(static_cast<int>(MessageType::GetTotal), 16);
	ASSERT_EQ(static_cast<int>(MessageType::ClearOpenOrders), 17);
	ASSERT_EQ(static_cast<int>(MessageType::ClearEveryonesOpenOrders), 18);
	ASSERT_EQ(static_cast<int>(MessageType::ClearAllEveryonesOpenOrders), 19);
	ASSERT_EQ(static_cast<int>(MessageType::NewOpenOrder), 20);
	ASSERT_EQ(static_cast<int>(MessageType::NewTrade), 21);
	ASSERT_EQ(static_cast<int>(MessageType::OrderFilled), 22);
	ASSERT_EQ(static_cast<int>(MessageType::NewFilledOrder), 23);
	ASSERT_EQ(static_cast<int>(MessageType::PartialFill), 24);
	ASSERT_EQ(static_cast<int>(MessageType::StopLimitTriggered), 25);
	ASSERT_EQ(static_cast<int>(MessageType::Quit), 26);
	ASSERT_EQ(static_cast<int>(MessageType::Last), 999999);
}
