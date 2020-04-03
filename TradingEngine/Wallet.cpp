#include "Wallet.h"

#include "Error.h"

#include <algorithm>
#include <boost/serialization/singleton.hpp>
#include <iterator>
#include <numeric>

BOOST_CLASS_EXPORT_IMPLEMENT(Wallet)

Wallet::Wallet(int32_t coinId) :
coinId(coinId) {
	if (coinId <= 0) {
		throw Error(Error::Type::InvalidCoinId);
	}
}

std::vector<Address>::iterator Wallet::AddAddress(const Address& address) {
	auto lb = findLbAddressFromUserId(address.GetUserId());
	if (lb != addresses.end() && lb->GetUserId() == address.GetUserId()) {
		throw Error(Error::Type::UserAlreadyExists);
	}

	lb = addresses.insert(lb, address);
	return lb;
}

std::vector<Address>::iterator Wallet::GetAddress(int32_t userId) {
	auto lb = findLbAddressFromUserId(userId);
	if (lb == addresses.end() || lb->GetUserId() != userId) {
		// Create an empty address.
		Address address(userId);
		lb = AddAddress(address);

		if (lb == addresses.end() || lb->GetUserId() != userId) {
			throw Error(Error::Type::CannotFindUser);
		}
	}

	return lb;
}

int32_t Wallet::GetCoinId() const {
	return coinId;
}

void Wallet::Deposit(int32_t userId, int64_t amount) {
	GetAddress(userId)->AddToTotalBalance(amount);
}

void Wallet::Withdraw(int32_t userId, int64_t amount) {
	GetAddress(userId)->RemoveFromTotalBalance(amount);
}

int64_t Wallet::GetTotal() const {
	// Loop over all addresses
	int64_t total = std::accumulate(addresses.cbegin(), addresses.cend(), (int64_t)0,
	[](int64_t amount, const Address& address) {
		return address.GetTotalBalance() + amount;
	});

	return total;
}

bool Wallet::Equals(const IWallet& inWallet) const {
	const auto& wallet = dynamic_cast<const Wallet&>(inWallet);
	return operator==(wallet);
}

bool Wallet::operator==(const Wallet& wallet) const {
	return (coinId == wallet.coinId && addresses == wallet.addresses);
}

std::vector<Address>::iterator Wallet::findLbAddressFromUserId(int32_t userId) {
	auto lb = std::lower_bound(addresses.begin(), addresses.end(), userId,
	[](const auto& address, int32_t userId) {
		return (address.GetUserId() < userId);
	});

	return lb;
}

const std::vector<Address>& Wallet::GetAddresses() const {
	return addresses;
}
