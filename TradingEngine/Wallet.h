#pragma once

#include "Address.h"
#include "IWallet.h"

#include <boost/serialization/export.hpp>
#include <boost/serialization/vector.hpp> // This gets removed from Eclipse formatting,
#include <boost/serialization/void_cast.hpp>
// but is needed to be able to serialize method to std::vector.

#include "serializer_defines.h"

#include <cstdint>
#include <vector>

SERIALIZE_HEADER(Wallet)

class User;

// Wallets contain everyones address for a specific coin
class Wallet : public IWallet {
public:
	SERIALIZE_FRIEND(Wallet)

	Wallet() = default; // For serializing
	Wallet(int32_t coinId);
	std::vector<Address>::iterator AddAddress(const Address& address) override;
	std::vector<Address>::iterator GetAddress(int32_t userId) override;
	int32_t GetCoinId() const override;
	void Deposit(int32_t userId, int64_t amount) override;
	void Withdraw(int32_t userId, int64_t amount) override;
	bool Equals(const IWallet& wallet) const override;

	bool operator==(const Wallet& wallet) const;
	int64_t GetTotal() const;

	// Just for testing
	const std::vector<Address>& GetAddresses() const;

private:
	int32_t coinId = -1;

	// This contains all the addresses for users, sorted by user id.
	// Not everyone will have one until there has been a buy/sell requiring them to.
	std::vector<Address> addresses;
	std::vector<Address>::iterator findLbAddressFromUserId(int32_t userId);
};

namespace boost::serialization {

template <class Archive>
void serialize(Archive& ar, Wallet& wallet, const unsigned int version) {
	boost::serialization::void_cast_register<Wallet, IWallet>();

	ar& wallet.addresses;
	ar& wallet.coinId;
}
}

BOOST_CLASS_EXPORT_KEY(Wallet)
