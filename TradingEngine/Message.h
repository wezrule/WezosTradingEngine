#pragma once

#include "MessageType.h"

#include <cstdint>
#include <string>

// This will be attached with Messages of MessageType::CancelAllOrders
struct Message {
	inline static std::string cancelOrders;

	MessageType messageType = MessageType::Last;
	int32_t id = 0; // This is set from Jason.. to uniquely identify messages

	union {
		int64_t stopPrice = -1;
		double feePercentage;
	};

	union {
		int64_t price;
		int32_t maxNumLimitOpenOrders;
	};

	// coinId/baseId are set to 0 explicitly so that the RPCMessageManager
	// knows if the message is Market related.
	int32_t baseId = 0;

	union {
		int32_t coinId = 0;
		int64_t tradeId;
	};

	union {
		int32_t userId;
		int32_t addressId;
		int64_t orderId;
		int64_t buyOrderId;
	};

	union {
		int64_t sellOrderId;
		int32_t orderType;
	};

	union {
		int64_t amount;
		int32_t maxNumStopLimitOpenOrders;
	};

	int64_t filled;

	union {
		bool isBuy;
		bool isRevertedDeposit = false;
	};

	int errorCode = 0;
	bool fullUpdate = true;

	inline bool isEmpty() const {
		return (messageType == MessageType::Last);
	}

	bool operator==(const Message& message) const {
		bool equal = false;
		switch (message.messageType) {
			case MessageType::Withdraw:
				equal = (coinId == message.coinId && userId == message.userId
				&& amount == message.amount);
				break;

			case MessageType::Deposit:
				equal = (coinId == message.coinId && userId == message.userId
				&& amount == message.amount && isRevertedDeposit == message.isRevertedDeposit);
				break;
			case MessageType::CancelOrder:
				equal = (coinId == message.coinId && baseId == message.baseId && isBuy == message.isBuy
				&& orderType == message.orderType && orderId == message.orderId
				&& price == message.price);
				break;
			case MessageType::CancelAllOrders:
				equal = (userId == message.userId);
				// Additionally the static cancelOrders needs to be checked..
				break;
			case MessageType::GetAvailable:
				equal = (coinId == message.coinId && userId == message.userId
				&& amount == message.amount);
				break;
			case MessageType::NewCoin:
				equal = (coinId == message.coinId);
				break;
			case MessageType::NewMarket:
				equal = (coinId == message.coinId && baseId == message.baseId);
				break;
			case MessageType::GetAmount:
			case MessageType::GetInOrder:
				equal = (coinId == message.coinId && userId == message.userId);
				break;
			case MessageType::SetInOrder:
				equal = (coinId == message.coinId && userId == message.userId
				&& amount == message.amount);
				break;
			case MessageType::GetTotal:
				equal = (coinId == message.coinId);
				break;
			case MessageType::SetFeePercentage:
				equal = (feePercentage == message.feePercentage);
				break;
			case MessageType::SetMaxNumLimitOpenOrders:
				equal = (maxNumLimitOpenOrders == message.maxNumLimitOpenOrders);
				break;
			case MessageType::SetMaxNumStopLimitOpenOrders:
				equal = (maxNumStopLimitOpenOrders == message.maxNumStopLimitOpenOrders);
				break;
			case MessageType::OrderFilled:
				equal = (orderId == message.orderId);
				break;
			case MessageType::NewTrade:
				equal = (buyOrderId == message.buyOrderId && sellOrderId == message.sellOrderId
				&& amount == message.amount && price == message.price
				&& tradeId == message.tradeId);
				break;
			case MessageType::NewOpenOrder:
				equal = (coinId == message.coinId && baseId == message.baseId
				&& userId == message.userId && isBuy == message.isBuy
				&& orderType == message.orderType && price == message.price
				&& stopPrice == message.stopPrice && amount == message.amount
				&& filled == message.filled);
				break;
			case MessageType::NewFilledOrder:
				equal = (coinId == message.coinId && baseId == message.baseId
				&& userId == message.userId && isBuy == message.isBuy
				&& orderType == message.orderType && amount == message.amount
				&& price == message.price);
				break;
			case MessageType::PartialFill:
				equal = (orderId == message.orderId && filled == message.filled);
				break;
			case MessageType::StopLimitTriggered:
				equal = (orderId == message.orderId && tradeId == message.tradeId);
				break;
			case MessageType::StopLimitOrder:
				equal = (stopPrice == message.stopPrice && coinId == message.coinId
				&& baseId == message.baseId && userId == message.userId
				&& isBuy == message.isBuy && amount == message.amount);
				break;
			case MessageType::LimitOrder:
				equal = (price == message.price && coinId == message.coinId && baseId == message.baseId
				&& userId == message.userId && isBuy == message.isBuy
				&& amount == message.amount);
				break;
			case MessageType::MarketOrder:
				equal = (coinId == message.coinId && baseId == message.baseId
				&& userId == message.userId && isBuy == message.isBuy
				&& amount == message.amount);
				break;
			case MessageType::ClearOpenOrders:
				equal = (userId == message.userId && coinId == message.coinId
				&& baseId == message.baseId);
				break;
			case MessageType::ClearEveryonesOpenOrders:
				equal = (coinId == message.coinId && baseId == message.baseId);
				break;
			case MessageType::ClearAllEveryonesOpenOrders:
				break;
			default:
				return false;
		};

		return equal && (errorCode == message.errorCode) && (id == message.id);
	}
};
