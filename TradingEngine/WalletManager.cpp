#include "WalletManager.h"

#include "Error.h"

#include <algorithm>
#include <iterator>

void WalletManager::AddWallet(const Wallet& wallet) {
	auto lb = findLbWalletFromCoinId(wallet.GetCoinId());
	if (lb != wallets.end() && lb->GetCoinId() == wallet.GetCoinId()) {
		throw Error(Error::Type::WalletAlreadyExists);
	}

	// Insert into sorted order
	lb = wallets.insert(lb, wallet);
}

std::vector<Wallet>::iterator WalletManager::GetWallet(int32_t coinId) {
	return findLbWalletFromCoinId(coinId);
}

bool WalletManager::AllWalletsEmpty() const {
	bool areEmpty = std::all_of(wallets.cbegin(), wallets.cend(), [](const auto& wallet) {
		return (wallet.GetTotal() == 0);
	});

	return areEmpty;
}

std::vector<Wallet>::iterator WalletManager::findLbWalletFromCoinId(int32_t coinId) {
	auto lb = std::lower_bound(wallets.begin(), wallets.end(), coinId,
	[](const auto& wallet, int32_t coinId) {
		return (wallet.GetCoinId() < coinId);
	});

	return lb;
}

const std::vector<Wallet>& WalletManager::GetWallets() const {
	return wallets;
}

bool WalletManager::operator==(const WalletManager& walletManager) const {
	return (wallets == walletManager.wallets);
}
