#include <TradingEngine/Error.h>
#include <gtest/gtest.h>

TEST(TestError, orderUnmodified) {
	// This confirms that the enum order is still correct.
	ASSERT_EQ(static_cast<int>(Error::Type::None), 0);
	ASSERT_EQ(static_cast<int>(Error::Type::UserAlreadyExists), 1);
	ASSERT_EQ(static_cast<int>(Error::Type::WalletAlreadyExists), 2);
	ASSERT_EQ(static_cast<int>(Error::Type::CannotFindUser), 3);
	ASSERT_EQ(static_cast<int>(Error::Type::MarketAlreadyExists), 4);
	ASSERT_EQ(static_cast<int>(Error::Type::CoinIdsSame), 5);
	ASSERT_EQ(static_cast<int>(Error::Type::InvalidCoinId), 6);
	ASSERT_EQ(static_cast<int>(Error::Type::TradeSameUser), 7);
	ASSERT_EQ(static_cast<int>(Error::Type::InvalidIdPrice), 8);
	ASSERT_EQ(static_cast<int>(Error::Type::InsufficientFunds), 9);
	ASSERT_EQ(static_cast<int>(Error::Type::InternalIteratorInvalid), 10);
	ASSERT_EQ(static_cast<int>(Error::Type::NoOrdersCannotPlaceStopLimit), 11);
	ASSERT_EQ(static_cast<int>(Error::Type::StopPriceTooHighLow), 12);
	ASSERT_EQ(static_cast<int>(Error::Type::Internal), 13);
	ASSERT_EQ(static_cast<int>(Error::Type::MarketOrderUnfilled), 14);
	ASSERT_EQ(static_cast<int>(Error::Type::InvalidListenerOperation), 15);
	ASSERT_EQ(static_cast<int>(Error::Type::IncompatibleOrders), 16);
	ASSERT_EQ(static_cast<int>(Error::Type::ReachedNumOpenOrders), 17);
	ASSERT_EQ(static_cast<int>(Error::Type::RPC_Failed), 18);
	ASSERT_EQ(static_cast<int>(Error::Type::FailedToReadDatabase), 19);
	ASSERT_EQ(static_cast<int>(Error::Type::Timeout), 20);
	ASSERT_EQ(static_cast<int>(Error::Type::RPCNotEnoughArguments), 21);
	ASSERT_EQ(static_cast<int>(Error::Type::InvalidMessageType), 22);

	ASSERT_EQ(static_cast<int>(Error::Type::FatalErrorUnknown), 10000);
	ASSERT_EQ(static_cast<int>(Error::Type::QueueDoesntExist), 10001);
	ASSERT_EQ(static_cast<int>(Error::Type::PurgeFailed), 10002);
	ASSERT_EQ(static_cast<int>(Error::Type::FailedSendingMessage), 10003);
	ASSERT_EQ(static_cast<int>(Error::Type::FailedDeletingMessage), 10004);
	ASSERT_EQ(static_cast<int>(Error::Type::FailedWritingMessage), 10005);
	ASSERT_EQ(static_cast<int>(Error::Type::CreateQueueFailed), 10006);

	ASSERT_EQ(static_cast<int>(Error::Type::Last), 9999999);
}
