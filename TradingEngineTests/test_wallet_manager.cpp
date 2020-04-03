#include <TradingEngine/Error.h>
#include <TradingEngine/Wallet.h>
#include <TradingEngine/WalletManager.h>
#include <gtest/gtest.h>
#include <iterator>

TEST(TestWalletManager, sameCoinId) {
	WalletManager walletManager;

	const int32_t coinId = 2;
	Wallet wallet{ coinId };
	walletManager.AddWallet(wallet);

	Wallet sameIdWallet{ coinId };
	ASSERT_THROW(walletManager.AddWallet(sameIdWallet), Error);
}

TEST(TestWalletManager, getWallet) {
	WalletManager walletManager;

	// Get wallet with only one in there
	const int32_t coinId = 34;
	Wallet wallet{ coinId };
	walletManager.AddWallet(wallet);

	ASSERT_EQ(wallet, *walletManager.GetWallet(coinId));

	// Get wallets with more than one
	const int32_t anotherCoinId = 35;
	Wallet anotherWallet{ anotherCoinId };
	walletManager.AddWallet(anotherWallet);

	ASSERT_EQ(anotherWallet, *walletManager.GetWallet(anotherCoinId));

	// Recheck
	ASSERT_EQ(wallet, *walletManager.GetWallet(coinId));
}
