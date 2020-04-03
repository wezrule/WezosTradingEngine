#include <TradingEngine/Address.h>
#include <gtest/gtest.h>

TEST(TestAddress, defaults) {
	Address address{ 5 };
	ASSERT_DOUBLE_EQ(address.GetAvailableBalance(), 0);
	ASSERT_DOUBLE_EQ(address.GetInOrder(), 0);
	ASSERT_EQ(address.GetUserId(), 5);
}

TEST(TestAddress, balance) {
	Address address{ 10 };
	//ASSERT_THROW(address.SetTotalBalance(-2.0), Error);
	ASSERT_DOUBLE_EQ(address.GetAvailableBalance(), 0);

	address.SetTotalBalance(200);
	ASSERT_DOUBLE_EQ(address.GetTotalBalance(), 200);

	address.SetTotalBalance(0);
	ASSERT_DOUBLE_EQ(address.GetTotalBalance(), 0);

	// Negative test
	ASSERT_NE(address.GetAvailableBalance(), 500);
}

TEST(TestAddress, inOrder) {
	Address address{ 5 };
	ASSERT_DOUBLE_EQ(address.GetInOrder(), 0);

	address.SetInOrder(1000);
	ASSERT_DOUBLE_EQ(address.GetInOrder(), 1000);

	address.SetInOrder(0);
	ASSERT_DOUBLE_EQ(address.GetInOrder(), 0);

	// Negative test
	ASSERT_NE(address.GetInOrder(), 500);
}
