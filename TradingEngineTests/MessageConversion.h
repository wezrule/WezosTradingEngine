#pragma once

#include <TradingEngine/Address.h>
#include <TradingEngine/CoinPair.h>
#include <TradingEngine/Fee.h>
#include <TradingEngine/MarketManager.h>
#include <TradingEngine/MessageType.h>
#include <TradingEngine/Wallet.h>
#include <TradingEngine/WalletManager.h>
#include <TradingEngine/market_helper.h>
#include <utility>

template <class Order, class Comp>
std::vector<Message> FromOrders(const CoinPair& coinPair, const OrderMap<Order, Comp>& orderMap,
bool isBuy) {
	std::vector<Message> messages;
	for (auto& [price, orders] : orderMap) {
		for (const auto& order : orders) {
			Message message;

			if constexpr (IsLimitOrder_v<Order>) {
				message.messageType = MessageType::LimitOrder;
			} else {
				message.messageType = MessageType::StopLimitOrder;
				message.stopPrice = order.GetActualPrice();
			}
			message.coinId = coinPair.GetCoinId();
			message.baseId = coinPair.GetBaseId();
			message.isBuy = isBuy;
			message.userId = order.GetUserId();
			message.amount = order.GetAmount();
			message.price = price;
			messages.push_back(message);
		}
	}

	return messages;
}

inline std::vector<Message> ToMessages(const Market& market) {
	std::vector<Message> messages;

	// Create the market
	Message message;
	message.messageType = MessageType::NewMarket;
	message.coinId = market.GetCoinPair().GetCoinId();
	message.baseId = market.GetCoinPair().GetBaseId();
	message.feePercentage = Fee::ConvertToFeePercent(market.GetConfig().feeDivision);
	message.maxNumLimitOpenOrders = market.GetConfig().maxNumLimitOpenOrders;
	message.maxNumStopLimitOpenOrders = market.GetConfig().maxNumStopLimitOpenOrders;
	messages.push_back(message);

	// Create limit/stop-limit orders
	std::vector<Message> buyLimitMessages = FromOrders(market.GetCoinPair(),
	market.GetBuyLimitOrderMap(), true);
	std::vector<Message> sellLimitMessages = FromOrders(market.GetCoinPair(),
	market.GetSellLimitOrderMap(), false);
	std::vector<Message> buyStopLimitMessages = FromOrders(market.GetCoinPair(),
	market.GetBuyStopLimitOrderMap(),
	true);
	std::vector<Message> sellStopLimitMessages = FromOrders(market.GetCoinPair(),
	market.GetSellStopLimitOrderMap(),
	false);

	messages.insert(messages.end(), buyLimitMessages.begin(), buyLimitMessages.end());
	messages.insert(messages.end(), sellLimitMessages.begin(), sellLimitMessages.end());
	messages.insert(messages.end(), buyStopLimitMessages.begin(), buyStopLimitMessages.end());
	messages.insert(messages.end(), sellStopLimitMessages.begin(), sellStopLimitMessages.end());

	return messages;
}

inline std::vector<Message> ToMessages(const MarketManager& marketManager) {
	std::vector<Message> messages;
	for (const auto& markets : marketManager.GetMarkets()) {
		for (const auto& market : markets.second) {
			std::vector<Message> messages1 = ToMessages(market);
			messages.insert(messages.end(), messages1.begin(), messages1.end());
		}
	}
	return messages;
}

inline std::vector<Message> ToMessages(const WalletManager& walletManager) {
	std::vector<Message> messages;
	Message message;
	for (const auto& wallet : walletManager.GetWallets()) {
		message.messageType = MessageType::NewCoin;
		message.coinId = wallet.GetCoinId();
		messages.push_back(message);

		for (const auto& address : wallet.GetAddresses()) {
			message.messageType = MessageType::Deposit;
			message.coinId = wallet.GetCoinId();
			message.userId = address.GetUserId();
			message.amount = address.GetTotalBalance();
			message.isRevertedDeposit = false;
			messages.push_back(message);
		}
	}
	return messages;
}
