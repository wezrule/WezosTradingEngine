#pragma once

#include "Address.h"

#include <boost/serialization/assume_abstract.hpp>
#include <cstdint>
#include <vector>

BOOST_SERIALIZATION_ASSUME_ABSTRACT(IWallet)

class IWallet {
public:
	virtual ~IWallet() = default;
	virtual std::vector<Address>::iterator AddAddress(const Address& address) = 0;
	virtual std::vector<Address>::iterator GetAddress(int32_t userId) = 0;
	virtual int32_t GetCoinId() const = 0;

	virtual void Deposit(int32_t userId, int64_t amount) = 0;
	virtual void Withdraw(int32_t userId, int64_t amount) = 0;
	virtual bool Equals(const IWallet& wallet) const = 0;

	bool operator==(const IWallet& wallet) const {
		return Equals(wallet);
	}
};
