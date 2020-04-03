#pragma once

#include "serializer_defines.h"

#include <cstdint>

SERIALIZE_HEADER(Address);

class Address {
public:
	SERIALIZE_FRIEND(Address);

	Address() = default; // For archiving
	Address(int32_t userId);

	void SetTotalBalance(int64_t balance);
	int64_t GetTotalBalance() const;
	void AddToTotalBalance(int64_t amount);
	void RemoveFromTotalBalance(int64_t amount);

	int64_t GetAvailableBalance() const;

	void SetInOrder(int64_t inOrder);
	int64_t GetInOrder() const;

	void AddToInOrder(int64_t amount);
	void RemoveFromInOrder(int64_t amount);

	int32_t GetUserId() const;

	bool operator==(const Address& address) const;

private:
	int32_t userId = 0;
	int64_t totalBalance = 0;
	int64_t numInOrder = 0;
};
