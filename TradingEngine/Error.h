#pragma once

#include <stdexcept>

class Error : public std::runtime_error {
public:
	// This should match that in admin.proto
	enum class Type {
		// DO NOT CHANGE ORDER!!
		None = 0,
		UserAlreadyExists,
		WalletAlreadyExists,
		CannotFindUser,
		MarketAlreadyExists,
		CoinIdsSame,
		InvalidCoinId,
		TradeSameUser,
		InvalidIdPrice,
		InsufficientFunds,
		InternalIteratorInvalid,
		NoOrdersCannotPlaceStopLimit,
		StopPriceTooHighLow,
		Internal,
		MarketOrderUnfilled,
		InvalidListenerOperation,
		IncompatibleOrders,
		ReachedNumOpenOrders,
		RPC_Failed,
		FailedToReadDatabase,
		Timeout,
		RPCNotEnoughArguments,
		InvalidMessageType,

		// Fatal errors start at 10000
		FatalErrorUnknown = 10000,
		QueueDoesntExist,
		PurgeFailed,
		FailedSendingMessage,
		FailedDeletingMessage,
		FailedWritingMessage,
		CreateQueueFailed,

		Last = 9999999
	};

	Error(Error::Type err) :
	std::runtime_error(""),
	err(err) {
	}

	Error(Error::Type err, const char* what) :
	std::runtime_error(what),
	err(err) {
	}

	Type GetType() const {
		return err;
	}

private:
	Type err;
};
