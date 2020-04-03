#include "TradingEngine.h"

#include "Address.h"
#include "CoinPair.h"
#include "Error.h"
#include "FatalError.h"
#include "Fee.h"
#include "Listener/Listener.h"
#include "Listener/ListenerOrder.h"
#include "Listener/Operation.h"
#include "Market.h"
#include "MessageType.h"
#include "Orders/OrderAction.h"
#include "Orders/OrderType.h"
#include "market_helper.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>

// Process a message and return a vector of output messages
std::vector<Message> TradingEngine::Process(const Message& message) {
	std::vector<Message> messages;

	try {
		switch (message.messageType) {
			case MessageType::MarketOrder:
				messages = ProcessOrder<MarketOrder>(message);
				break;

			case MessageType::LimitOrder:
				messages = ProcessOrder<LimitOrder>(message);
				break;

			case MessageType::StopLimitOrder:
				messages = ProcessOrder<StopLimitOrder>(message);
				break;

			case MessageType::CancelOrder:
				if (static_cast<OrderType>(message.orderType) == OrderType::Limit) {
					CancelOrder<LimitOrder>(message);
				} else if (static_cast<OrderType>(message.orderType) == OrderType::StopLimit) {
					CancelOrder<StopLimitOrder>(message);
				}

				if (message.fullUpdate) {
					messages.push_back(message);
				}
				break;
			case MessageType::CancelAllOrders: {
				auto allCancelledOrders = marketManager.CancelAll(message.userId, walletManager);

				std::ostringstream ss;
				for (auto& [coinPair, cancelledIds] : allCancelledOrders) {
					for (auto& id : cancelledIds) {
						ss << coinPair.GetCoinId() << "_" << coinPair.GetBaseId() << "_" << id << " ";
					}
				}

				Message::cancelOrders = ss.str();
				if (!Message::cancelOrders.empty()) {
					Message::cancelOrders.erase(Message::cancelOrders.end() - 1); // Remove last space from string.
				}

				if (message.fullUpdate) {
					messages.push_back(message);
				}
				break;
			}
			case MessageType::Deposit:
				walletManager.GetWallet(message.coinId)->Deposit(message.userId, message.amount);
				if (message.fullUpdate) {
					messages.push_back(message);
				}
				break;
			case MessageType::Withdraw:
				walletManager.GetWallet(message.coinId)->Withdraw(message.userId, message.amount);
				if (message.fullUpdate) {
					messages.push_back(message);
				}
				break;
			case MessageType::NewCoin:
				walletManager.AddWallet({ message.coinId });
				messages.push_back(message);
				break;
			case MessageType::NewMarket: {
				CoinPair coinPair = { message.coinId, message.baseId };
				auto fee = Fee::ConvertToDivisibleFee(message.feePercentage);

				MarketConfig marketConfig{ fee, message.maxNumLimitOpenOrders, message.maxNumStopLimitOpenOrders };

				Market market{ std::make_unique<Listener>(), coinPair, marketConfig };
				marketManager.AddMarket(std::move(market));
				messages.push_back(message);
				break;
			}

			// Admin setters don't need to output messages
			case MessageType::SetInOrder: {
				auto address = walletManager.GetWallet(message.coinId)->GetAddress(message.userId);
				address->SetInOrder(message.amount);
				break;
			}
			case MessageType::SetFeePercentage:
				// Currently does it for all markets
				marketManager.SetFees(message.feePercentage);
				break;
			case MessageType::SetMaxNumLimitOpenOrders:
				// Currently does it for all markets
				marketManager.SetMaxNumLimitOpenOrders(message.maxNumLimitOpenOrders);
				break;
			case MessageType::SetMaxNumStopLimitOpenOrders:
				// Currently does it for all markets
				marketManager.SetMaxNumStopLimitOpenOrders(message.maxNumStopLimitOpenOrders);
				break;

			case MessageType::GetAmount: {
				Message outputMessage;
				auto address = walletManager.GetWallet(message.coinId)->GetAddress(message.userId);
				outputMessage.amount = address->GetTotalBalance();
				messages.push_back(outputMessage);
				break;
			}
			case MessageType::GetAvailable: {
				Message outputMessage;
				auto address = walletManager.GetWallet(message.coinId)->GetAddress(message.userId);
				auto total = address->GetTotalBalance();
				auto inOrder = address->GetInOrder();
				outputMessage.amount = total - inOrder;
				messages.push_back(outputMessage);
				break;
			}
			case MessageType::GetInOrder: {
				Message outputMessage;
				auto address = walletManager.GetWallet(message.coinId)->GetAddress(message.userId);
				outputMessage.amount = address->GetInOrder();
				messages.push_back(outputMessage);
				break;
			}
			case MessageType::GetTotal: {
				Message outputMessage;
				outputMessage.amount = walletManager.GetWallet(message.coinId)->GetTotal();
				messages.push_back(outputMessage);
				break;
			}
			case MessageType::ClearOpenOrders: {
				auto market = marketManager.GetMarket({ message.coinId, message.baseId });
				auto userId = message.userId;

				auto coinWallet = walletManager.GetWallet(market->GetCoinPair().GetCoinId());
				coinWallet->GetAddress(userId)->SetInOrder(0);

				auto baseWallet = walletManager.GetWallet(market->GetCoinPair().GetBaseId());
				baseWallet->GetAddress(userId)->SetInOrder(0);

				market->CancelAll(message.userId);
				break;
			}
			case MessageType::ClearEveryonesOpenOrders: {
				auto market = marketManager.GetMarket({ message.coinId, message.baseId });

				auto coinWallet = walletManager.GetWallet(market->GetCoinPair().GetCoinId());
				auto baseWallet = walletManager.GetWallet(market->GetCoinPair().GetBaseId());

				for (auto& address : coinWallet->GetAddresses()) {
					const_cast<Address&>(address).SetInOrder(0);
				}

				for (auto& address : baseWallet->GetAddresses()) {
					const_cast<Address&>(address).SetInOrder(0);
				}

				market->CancelAll();
				break;
			}
			case MessageType::ClearAllEveryonesOpenOrders: {
				marketManager.CancelAll(walletManager);
				break;
			}

			default:
				throw Error(Error::Type::InvalidMessageType);
		}
		// We do not handle FatalErrors here
	} catch (const Error& error) { // These are expected errors
		messages.clear(); // Should be empty already, but make sure..
		Message errorMessage = message;
		errorMessage.errorCode = static_cast<int>(error.GetType());
		messages.push_back(errorMessage);
	}

	return messages;
}

MarketWallets TradingEngine::GetMarketWallets(const Message& message) {
	auto coinWallet = walletManager.GetWallet(message.coinId);
	auto baseWallet = walletManager.GetWallet(message.baseId);
	return MarketWallets{ &*coinWallet, &*baseWallet };
}

template <typename T>
std::vector<Message> TradingEngine::ProcessOrder(const Message& message) {
	std::vector<Message> messages;

	auto market = marketManager.GetMarket({ message.coinId, message.baseId });
	auto marketWallets = GetMarketWallets(message);
	auto order = CreateOrder<T>(message);

	if (message.isBuy) {
		market->NewProcess<OrderAction::Buy>(order, &marketWallets);
	} else {
		market->NewProcess<OrderAction::Sell>(order, &marketWallets);
	}

	auto& listener = market->GetListener();

	const auto& operations = listener.GetOperations();
	for (auto& operation : operations) {
		Message outputMessage;
		switch (operation.type) {
			case Operation::Type::OrderFilled:
				outputMessage.messageType = MessageType::OrderFilled;
				outputMessage.orderId = operation.listenerOrder.orderId;
				break;
			case Operation::Type::NewTrade:
				outputMessage.messageType = MessageType::NewTrade;
				outputMessage.buyOrderId = operation.listenerOrder.buyOrderId;
				outputMessage.sellOrderId = operation.listenerOrder.sellOrderId;
				outputMessage.amount = operation.listenerOrder.amount;
				outputMessage.price = operation.listenerOrder.price;
				outputMessage.tradeId = operation.listenerOrder.tradeId;
				break;
			case Operation::Type::NewOpenOrder:
				outputMessage.messageType = MessageType::NewOpenOrder;
				outputMessage.coinId = market->GetCoinPair().GetCoinId();
				outputMessage.baseId = market->GetCoinPair().GetBaseId();
				outputMessage.userId = operation.listenerOrder.userId;
				outputMessage.isBuy = (operation.action == OrderAction::Buy);
				outputMessage.orderType = static_cast<int>(operation.listenerOrder.orderType);
				outputMessage.price = operation.listenerOrder.price;
				outputMessage.stopPrice = operation.listenerOrder.actualPrice;
				outputMessage.amount = operation.listenerOrder.amount;
				outputMessage.filled = operation.listenerOrder.filled;
				break;
			case Operation::Type::NewFilledOrder:
				outputMessage.messageType = MessageType::NewFilledOrder;
				outputMessage.coinId = market->GetCoinPair().GetCoinId();
				outputMessage.baseId = market->GetCoinPair().GetBaseId();
				outputMessage.userId = operation.listenerOrder.userId;
				outputMessage.isBuy = (operation.action == OrderAction::Buy);
				outputMessage.orderType = static_cast<int>(operation.listenerOrder.orderType);
				outputMessage.amount = operation.listenerOrder.amount;
				outputMessage.price = operation.listenerOrder.price;
				break;
			case Operation::Type::PartialFill:
				outputMessage.messageType = MessageType::PartialFill;
				outputMessage.orderId = operation.listenerOrder.orderId;
				outputMessage.filled = operation.listenerOrder.filled;
				break;
			case Operation::Type::StopLimitTriggered:
				outputMessage.messageType = MessageType::StopLimitTriggered;
				outputMessage.orderId = operation.listenerOrder.orderId;
				outputMessage.tradeId = operation.listenerOrder.triggeredOrderId;
				break;
			default:
				throw Error(Error::Type::InvalidListenerOperation, "This operation is not supported");
		}

		messages.push_back(std::move(outputMessage));
	}

	listener.ClearOperations();
	return messages;
}

template <typename Order>
void TradingEngine::CancelOrder(const Message& message) {
	auto market = marketManager.GetMarket({ message.coinId, message.baseId });

	auto coinWallet = walletManager.GetWallet(market->GetCoinPair().GetCoinId());
	auto baseWallet = walletManager.GetWallet(market->GetCoinPair().GetBaseId());

	MarketWallets marketWallets{ &*coinWallet, &*baseWallet };

	if (message.isBuy) {
		market->CancelOrder<OrderAction::Buy, Order>(message.orderId, message.price,
		&marketWallets);
	} else {
		market->CancelOrder<OrderAction::Sell, Order>(message.orderId, message.price,
		&marketWallets);
	}
}

bool TradingEngine::operator==(const TradingEngine& TradingEngine) const {
	return marketManager == TradingEngine.marketManager
	&& walletManager == TradingEngine.walletManager;
}

template <>
OrderContainer<LimitOrder> TradingEngine::CreateOrder(const Message& message) {
	return OrderContainer<LimitOrder>({ message.userId, message.amount, 0 }, message.price);
}

template <>
OrderContainer<StopLimitOrder> TradingEngine::CreateOrder(const Message& message) {
	return OrderContainer<StopLimitOrder>(
	{ message.userId, message.amount, 0, message.stopPrice }, message.price);
}

template <>
OrderContainer<MarketOrder> TradingEngine::CreateOrder(const Message& message) {
	return OrderContainer<MarketOrder>({ message.userId, message.amount }, 0);
}
