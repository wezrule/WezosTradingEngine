#pragma once

#include <TradingEngine/Address.h>
#include <TradingEngine/IWallet.h>
#include <TradingEngine/Units.h>
#include <cstdint>
#include <vector>

class StubWallet : public IWallet {
	std::vector<Address> addresses;

public:
	StubWallet() {
		Address address(0);
		address.SetTotalBalance(Units::ExToIn(10000000000.0));
		addresses.push_back(address);
	}

	std::vector<Address>::iterator AddAddress(const Address& address) override {
		return addresses.begin();
	}
	std::vector<Address>::iterator GetAddress(int32_t userId) override {
		return addresses.begin();
	}
	int32_t GetCoinId() const override { return 0; }
	void Deposit(int32_t userId, int64_t amount) override {}
	void Withdraw(int32_t userId, int64_t amount) override {}
	bool Equals(const IWallet& wallet) const override { return false; }
};
