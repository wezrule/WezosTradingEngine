#include <TradingEngine/Address.h>
#include <TradingEngine/Error.h>
#include <TradingEngine/Wallet.h>
#include <gtest/gtest.h>
#include <iterator>

TEST(TestWallet, invalidId) {
	ASSERT_THROW(Wallet{ -1 }, Error);
}

TEST(TestWallet, defaults) {
	Wallet wallet{ 20 };
	ASSERT_EQ(wallet.GetCoinId(), 20);
}

TEST(TestWallet, sameUserId) {
	Wallet wallet{ 5 };

	const int32_t userId = 10003;
	Address address{ userId };
	wallet.AddAddress(address);

	Address addressWithSameUserId{ userId };
	ASSERT_THROW(wallet.AddAddress(addressWithSameUserId), Error);
}

TEST(TestWallet, getAddress) {
	Wallet wallet{ 5 };

	// Empty wallet
	ASSERT_NO_THROW(wallet.GetAddress(1));

	// One address
	const int32_t userId = 500;
	Address address{ userId };
	wallet.AddAddress(address);

	ASSERT_EQ(address, *wallet.GetAddress(userId));

	// Multiple addresses
	const int32_t anotherUserId = 1000;
	Address anotherAddress{ anotherUserId };
	wallet.AddAddress(anotherAddress);

	ASSERT_EQ(anotherAddress, *wallet.GetAddress(anotherUserId));

	// Recheck that this works
	ASSERT_EQ(address, *wallet.GetAddress(userId));
}
