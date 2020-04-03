#include "Address.h"

Address::Address(int32_t userId) :
userId(userId) {
}

void Address::SetTotalBalance(int64_t balance) {
	totalBalance = balance;
}

int64_t Address::GetAvailableBalance() const {
	return totalBalance - numInOrder;
}

int64_t Address::GetTotalBalance() const {
	return totalBalance;
}

void Address::AddToTotalBalance(int64_t amount) {
	totalBalance += amount;
}

void Address::RemoveFromTotalBalance(int64_t amount) {
	totalBalance -= amount;
}

void Address::SetInOrder(int64_t inOrder) {
	numInOrder = inOrder;
}

int64_t Address::GetInOrder() const {
	return numInOrder;
}

void Address::AddToInOrder(int64_t amount) {
	numInOrder += amount;
}

void Address::RemoveFromInOrder(int64_t amount) {
	numInOrder -= amount;
}

int32_t Address::GetUserId() const {
	return userId;
}

bool Address::operator==(const Address& address) const {
	return (userId == address.userId && totalBalance == address.totalBalance
	&& numInOrder == address.numInOrder);
}
