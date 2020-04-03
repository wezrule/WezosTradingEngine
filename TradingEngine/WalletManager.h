#pragma once

#include "Wallet.h"
#include "serializer_defines.h"

#include <cstdint>
#include <vector>

SERIALIZE_HEADER(WalletManager)

class WalletManager {
public:
	SERIALIZE_FRIEND(WalletManager)

	WalletManager() = default; // For serializing
	void AddWallet(const Wallet& wallet);
	std::vector<Wallet>::iterator GetWallet(int32_t coinId);
	bool operator==(const WalletManager& walletManager) const;
	int64_t GetTotal() const;
	bool AllWalletsEmpty() const;

	// Just for testing
	const std::vector<Wallet>& GetWallets() const;

private:
	// This contains a sorted collection of the wallets, by wallet id.
	std::vector<Wallet> wallets;
	std::vector<Wallet>::iterator findLbWalletFromCoinId(int32_t coinId);
};
