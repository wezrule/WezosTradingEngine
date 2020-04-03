#include "message_conversion_testing_helper.h"

#include <TradingEngine/Address.h>
#include <TradingEngine/CoinPair.h>
#include <TradingEngine/Error.h>
#include <TradingEngine/Fee.h>
#include <TradingEngine/Market.h>
#include <TradingEngine/MarketManager.h>
#include <TradingEngine/Message.h>
#include <TradingEngine/MessageType.h>
#include <TradingEngine/Orders/OrderType.h>
#include <TradingEngine/TradingEngine.h>
#include <TradingEngine/Units.h>
#include <TradingEngine/Wallet.h>
#include <TradingEngine/WalletManager.h>
#include <TradingEngine/market_helper.h>
#include <cstdint>
#include <gtest/gtest.h>
#include <vector>

class TradingEngineProcessing : public ::testing::Test {
protected:
	TradingEngine tradingEngine;

	TradingEngineProcessing() {
		auto messages = CreateSimpleMessages();

		for (auto& message : messages) {
			(void)tradingEngine.Process(message);
			// Don't bother checking these output messages here, have a separate test to check them
		}
	}
};

void CompareOtherMarket(MarketManager& marketManager, WalletManager& walletManager) {
	// Confirm wallets are same
	// confirm market is same

	auto baseWallet = CreateBaseWallet(1);
	auto coinWallet = CreateWallet(1);
	MarketWallets marketWallets{ &baseWallet, &coinWallet };
	auto market = CreateMarket(&marketWallets, 1);
	auto coinPair = CreateAnotherCoinPair();

	ASSERT_EQ(market, *marketManager.GetMarket(coinPair));

	// Confirm wallets are unchanged
	auto CheckBalances = [&walletManager](const auto& coinId, int32_t id) {
		auto address = walletManager.GetWallet(coinId)->GetAddress(id);
		ASSERT_EQ(address->GetInOrder(), GetInOrder());
		ASSERT_EQ(address->GetTotalBalance(), GetTotalBalance());
	};

	CheckBalances(coinPair.GetBaseId(), 6);
	CheckBalances(coinPair.GetCoinId(), 7);
}

TEST_F(TradingEngineProcessing, Error) {
	Message message;
	message.messageType = MessageType::NewMarket;
	message.coinId = 1;
	message.baseId = 1;
	auto outputMessages = tradingEngine.Process(message);
	ASSERT_EQ(outputMessages.size(), 1u);
	ASSERT_EQ(outputMessages.front().errorCode, static_cast<int>(Error::Type::CoinIdsSame));
}

TEST_F(TradingEngineProcessing, UnexpectedError) {
	Message message;

	// Use a message which doesn't exist
	message.messageType = static_cast<MessageType>(10000);
	message.userId = 8;
	auto outputMessages = tradingEngine.Process(message);
	ASSERT_EQ(outputMessages.size(), 1u);
	ASSERT_EQ(outputMessages.front().errorCode,
	static_cast<int>(Error::Type::InvalidMessageType));
}

TEST_F(TradingEngineProcessing, MarketOrderBuy) {
	Message message;
	message.messageType = MessageType::MarketOrder;
	message.userId = BuyUserId();
	message.isBuy = true;
	message.coinId = 3;
	message.baseId = 1;
	message.amount = Units::ExToIn(1.0);
	auto outputMessages = tradingEngine.Process(message);

	/* TODO: These tests are failing
  Message outputMessage;
  outputMessage.messageType = MessageType::NewTrade;
  outputMessage.buyOrderId = 9;
  outputMessage.sellOrderId = 3;
  outputMessage.amount = Units::ExToIn(1.0);
  outputMessage.price = Units::ExToIn(0.6);
  outputMessage.tradeId = 1;
  ASSERT_EQ(outputMessage, outputMessages.front());

  outputMessage.messageType = MessageType::PartialFill;
  outputMessage.orderId = 3;
  outputMessage.filled = Units::ExToIn(1.0);
  ASSERT_EQ(outputMessage, outputMessages[1]);

  outputMessage.messageType = MessageType::NewFilledOrder;
  outputMessage.coinId = 3;
  outputMessage.baseId = 1;
  outputMessage.userId = 6;
  outputMessage.isBuy = true;
  outputMessage.orderType = static_cast<int>(OrderType::Market);
  outputMessage.amount = Units::ExToIn(1.0);
  outputMessage.price = -1;
  ASSERT_EQ(outputMessage, outputMessages[2]);

  outputMessage.messageType = MessageType::StopLimitTriggered;
  outputMessage.orderId = 5;
  outputMessage.tradeId = 1;
  ASSERT_EQ(outputMessage, outputMessages[3]);

  outputMessage.messageType = MessageType::NewTrade;
  outputMessage.buyOrderId = 5;
  outputMessage.sellOrderId = 3;
  outputMessage.amount = Units::ExToIn(199.0);
  outputMessage.price = Units::ExToIn(0.6);
  outputMessage.tradeId = 2;
  ASSERT_EQ(outputMessage, outputMessages[4]);

  outputMessage.messageType = MessageType::OrderFilled;
  outputMessage.orderId = 3;
  ASSERT_EQ(outputMessage, outputMessages[5]);

  outputMessage.messageType = MessageType::NewOpenOrder;
  outputMessage.coinId = 3;
  outputMessage.baseId = 1;
  outputMessage.userId = 6;
  outputMessage.isBuy = true;
  outputMessage.orderType = static_cast<int>(OrderType::Limit);
  outputMessage.price = Units::ExToIn(0.6);
  outputMessage.stopPrice = -1;
  outputMessage.amount = Units::ExToIn(200.0);
  outputMessage.filled = Units::ExToIn(199.0);
  ASSERT_EQ(outputMessage, outputMessages.back());

  ASSERT_EQ(outputMessages.size(), 7u);
  CompareOtherMarket(tradingEngine.GetMarketManager(), tradingEngine.GetWalletManager());
  */
}

TEST_F(TradingEngineProcessing, MarketOrderSell) {
	Message message;
	message.messageType = MessageType::MarketOrder;
	message.userId = SellUserId();
	message.isBuy = false;
	message.coinId = 3;
	message.baseId = 1;
	message.amount = Units::ExToIn(2.0);
	auto outputMessages = tradingEngine.Process(message);

	/*
  Message outputMessage;
  outputMessage.messageType = MessageType::NewTrade;
  outputMessage.buyOrderId = 1;
  outputMessage.sellOrderId = 9;
  outputMessage.amount = Units::ExToIn(2.0);
  outputMessage.price = Units::ExToIn(0.5);
  outputMessage.tradeId = 1;
  ASSERT_EQ(outputMessage, outputMessages.front());

  outputMessage.messageType = MessageType::PartialFill;
  outputMessage.orderId = 1;
  outputMessage.filled = Units::ExToIn(2.0);
  ASSERT_EQ(outputMessage, outputMessages[1]);

  outputMessage.messageType = MessageType::NewFilledOrder;
  outputMessage.coinId = 3;
  outputMessage.baseId = 1;
  outputMessage.userId = 7;
  outputMessage.isBuy = false;
  outputMessage.orderType = static_cast<int>(OrderType::Market);
  outputMessage.amount = Units::ExToIn(2.0);
  outputMessage.price = -1;
  ASSERT_EQ(outputMessage, outputMessages[2]);

  outputMessage.messageType = MessageType::StopLimitTriggered;
  outputMessage.orderId = 7;
  outputMessage.tradeId = 1;
  ASSERT_EQ(outputMessage, outputMessages[3]);

  outputMessage.messageType = MessageType::NewTrade;
  outputMessage.buyOrderId = 1;
  outputMessage.sellOrderId = 7;
  outputMessage.amount = Units::ExToIn(198.0);
  outputMessage.price = Units::ExToIn(0.5);
  outputMessage.tradeId = 2;
  ASSERT_EQ(outputMessage, outputMessages[4]);

  outputMessage.messageType = MessageType::OrderFilled;
  outputMessage.orderId = 1;
  ASSERT_EQ(outputMessage, outputMessages[5]);

  outputMessage.messageType = MessageType::NewOpenOrder;
  outputMessage.coinId = 3;
  outputMessage.baseId = 1;
  outputMessage.userId = 7;
  outputMessage.isBuy = false;
  outputMessage.orderType = static_cast<int>(OrderType::Limit);
  outputMessage.price = Units::ExToIn(0.5);
  outputMessage.stopPrice = -1;
  outputMessage.amount = Units::ExToIn(200.0);
  outputMessage.filled = Units::ExToIn(198.0);
  ASSERT_EQ(outputMessage, outputMessages.back());

  ASSERT_EQ(outputMessages.size(), 7u);
  CompareOtherMarket(tradingEngine.GetMarketManager(), tradingEngine.GetWalletManager());*/
}

TEST_F(TradingEngineProcessing, LimitOrderBuy) {
	Message message;
	message.messageType = MessageType::LimitOrder;
	message.isBuy = true;
	message.coinId = 3;
	message.userId = BuyUserId();
	message.baseId = 1;
	message.amount = Units::ExToIn(1.0);
	message.price = Units::ExToIn(0.3);
	auto outputMessages = tradingEngine.Process(message);

	ASSERT_EQ(outputMessages.size(), 1u);
	ASSERT_EQ(outputMessages.front().errorCode, 0);

	Message outputMessage = message;
	outputMessage.messageType = MessageType::NewOpenOrder;
	outputMessage.orderType = static_cast<int>(OrderType::Limit);
	outputMessage.filled = Units::ExToIn(0.0);
	ASSERT_EQ(outputMessage, outputMessages.front());
	CompareOtherMarket(tradingEngine.GetMarketManager(), tradingEngine.GetWalletManager());
}

TEST_F(TradingEngineProcessing, LimitOrderSell) {
	Message message;
	message.messageType = MessageType::LimitOrder;
	message.isBuy = false;
	message.userId = SellUserId();
	message.coinId = 3;
	message.baseId = 1;
	message.userId = SellUserId();
	message.amount = Units::ExToIn(1.0);
	message.price = Units::ExToIn(0.8);
	auto outputMessages = tradingEngine.Process(message);

	ASSERT_EQ(outputMessages.size(), 1u);
	ASSERT_EQ(outputMessages.front().errorCode, 0);

	Message outputMessage = message;
	outputMessage.messageType = MessageType::NewOpenOrder;
	outputMessage.orderType = static_cast<int>(OrderType::Limit);
	outputMessage.filled = Units::ExToIn(0.0);
	ASSERT_EQ(outputMessage, outputMessages.front());
	CompareOtherMarket(tradingEngine.GetMarketManager(), tradingEngine.GetWalletManager());
}

TEST_F(TradingEngineProcessing, StopLimitOrderBuy) {
	Message message;
	message.messageType = MessageType::StopLimitOrder;
	message.isBuy = true;
	message.userId = BuyUserId();
	message.coinId = 3;
	message.baseId = 1;
	message.amount = Units::ExToIn(1.0);
	message.price = Units::ExToIn(0.8);
	message.stopPrice = Units::ExToIn(0.8);
	auto outputMessages = tradingEngine.Process(message);

	ASSERT_EQ(outputMessages.size(), 1u);
	ASSERT_EQ(outputMessages.front().errorCode, 0);

	Message outputMessage = message;
	outputMessage.messageType = MessageType::NewOpenOrder;
	outputMessage.orderType = static_cast<int>(OrderType::StopLimit);
	outputMessage.filled = Units::ExToIn(0.0);
	ASSERT_EQ(outputMessage, outputMessages.front());
	CompareOtherMarket(tradingEngine.GetMarketManager(), tradingEngine.GetWalletManager());
}

TEST_F(TradingEngineProcessing, StopLimitOrderSell) {
	Message message;
	message.messageType = MessageType::StopLimitOrder;
	message.isBuy = false;
	message.coinId = 3;
	message.baseId = 1;
	message.userId = 7;
	message.amount = Units::ExToIn(1.0);
	message.price = Units::ExToIn(0.3);
	message.stopPrice = Units::ExToIn(0.3);
	auto outputMessages = tradingEngine.Process(message);

	ASSERT_EQ(outputMessages.size(), 1u);
	ASSERT_EQ(outputMessages.front().errorCode, 0);

	Message outputMessage = message;
	outputMessage.messageType = MessageType::NewOpenOrder;
	outputMessage.orderType = static_cast<int>(OrderType::StopLimit);
	outputMessage.filled = Units::ExToIn(0.0);
	ASSERT_EQ(outputMessage, outputMessages.front());
	CompareOtherMarket(tradingEngine.GetMarketManager(), tradingEngine.GetWalletManager());
}

TEST_F(TradingEngineProcessing, CancelOrder) {
	Message message;
	message.messageType = MessageType::CancelOrder;
	message.coinId = 3;
	message.baseId = 1;
	message.isBuy = false;
	message.orderType = static_cast<int32_t>(OrderType::StopLimit);
	message.orderId = 7;
	message.price = Units::ExToIn(0.5);
	auto outputMessages = tradingEngine.Process(message);
	ASSERT_EQ(outputMessages.size(), 1u);
	ASSERT_EQ(outputMessages.front().errorCode, 0);
	ASSERT_EQ(message, outputMessages.front());

	auto market = tradingEngine.GetMarketManager().GetMarket(CreateCoinPair());
	ASSERT_EQ(Flatten(market->GetSellStopLimitOrderMap()).size(), 1u);

	// Check wallet
	auto& walletManager = tradingEngine.GetWalletManager();
	ASSERT_EQ(walletManager.GetWallet(message.coinId)->GetAddress(7)->GetInOrder(),
	Units::ExToIn(600.0));
	CompareOtherMarket(tradingEngine.GetMarketManager(), tradingEngine.GetWalletManager());
}

TEST_F(TradingEngineProcessing, CancelAllOrders) {
	// Clear id 7, which just consists of sell orders..
	Message message;
	message.messageType = MessageType::CancelAllOrders;
	message.userId = 7;
	auto outputMessages = tradingEngine.Process(message);
	ASSERT_EQ(outputMessages.size(), 1u);
	ASSERT_EQ(outputMessages.front().errorCode, 0);
	ASSERT_EQ(message, outputMessages.front());
	ASSERT_EQ("4_2_3 4_2_4 4_2_7 4_2_8 3_1_3 3_1_4 3_1_7 3_1_8", Message::cancelOrders);

	auto market = tradingEngine.GetMarketManager().GetMarket(CreateCoinPair());
	ASSERT_EQ(Flatten(market->GetSellStopLimitOrderMap()).size(), 0u);
	ASSERT_EQ(Flatten(market->GetSellLimitOrderMap()).size(), 0u);

	auto otherMarket = tradingEngine.GetMarketManager().GetMarket(CreateAnotherCoinPair());
	ASSERT_EQ(Flatten(otherMarket->GetSellStopLimitOrderMap()).size(), 0u);
	ASSERT_EQ(Flatten(otherMarket->GetSellLimitOrderMap()).size(), 0u);

	// Check wallet
	auto& walletManager = tradingEngine.GetWalletManager();
	ASSERT_EQ(walletManager.GetWallet(CreateCoinPair().GetCoinId())->GetAddress(7)->GetInOrder(),
	Units::ExToIn(0.0));
	ASSERT_EQ(walletManager.GetWallet(CreateAnotherCoinPair().GetCoinId())->GetAddress(7)->GetInOrder(),
	Units::ExToIn(0.0));

	// Clear id 6, which just consists of buy orders..
	message.userId = 6;
	outputMessages = tradingEngine.Process(message);
	ASSERT_EQ(outputMessages.size(), 1u);
	ASSERT_EQ(outputMessages.front().errorCode, 0);
	ASSERT_EQ(message, outputMessages.front());
	ASSERT_EQ("4_2_1 4_2_2 4_2_5 4_2_6 3_1_1 3_1_2 3_1_5 3_1_6", Message::cancelOrders);

	ASSERT_EQ(Flatten(market->GetBuyStopLimitOrderMap()).size(), 0u);
	ASSERT_EQ(Flatten(market->GetBuyLimitOrderMap()).size(), 0u);
	ASSERT_EQ(Flatten(otherMarket->GetSellStopLimitOrderMap()).size(), 0u);
	ASSERT_EQ(Flatten(otherMarket->GetSellLimitOrderMap()).size(), 0u);

	ASSERT_EQ(walletManager.GetWallet(CreateCoinPair().GetBaseId())->GetAddress(6)->GetInOrder(),
	Units::ExToIn(0.0));
	ASSERT_EQ(walletManager.GetWallet(CreateAnotherCoinPair().GetBaseId())->GetAddress(6)->GetInOrder(),
	Units::ExToIn(0.0));
}

TEST_F(TradingEngineProcessing, ClearOpenOrders) {
	// Market, user
	Message message;
	message.messageType = MessageType::ClearOpenOrders;
	message.userId = 7;
	message.coinId = CreateCoinPair().GetCoinId();
	message.baseId = CreateCoinPair().GetBaseId();
	auto outputMessages = tradingEngine.Process(message);

	auto market = tradingEngine.GetMarketManager().GetMarket(CreateCoinPair());
	ASSERT_EQ(Flatten(market->GetSellStopLimitOrderMap()).size(), 0u);
	ASSERT_EQ(Flatten(market->GetSellLimitOrderMap()).size(), 0u);

	// Check wallet
	auto& walletManager = tradingEngine.GetWalletManager();
	const auto& coinPair = CreateCoinPair();
	ASSERT_EQ(walletManager.GetWallet(coinPair.GetCoinId())->GetAddress(7)->GetInOrder(),
	Units::ExToIn(0.0));

	// Should be untouched
	ASSERT_EQ(walletManager.GetWallet(coinPair.GetBaseId())->GetAddress(6)->GetInOrder(),
	Units::ExToIn(800.0));
	CompareOtherMarket(tradingEngine.GetMarketManager(), tradingEngine.GetWalletManager());
}

TEST_F(TradingEngineProcessing, ClearEveryonesOpenOrders) {
	const auto& coinPair = CreateCoinPair();
	Message message;
	message.messageType = MessageType::ClearEveryonesOpenOrders;
	message.coinId = coinPair.GetCoinId();
	message.baseId = coinPair.GetBaseId();
	auto outputMessages = tradingEngine.Process(message);

	auto market = tradingEngine.GetMarketManager().GetMarket(CreateCoinPair());
	ASSERT_EQ(Flatten(market->GetSellStopLimitOrderMap()).size(), 0u);
	ASSERT_EQ(Flatten(market->GetSellLimitOrderMap()).size(), 0u);
	ASSERT_EQ(Flatten(market->GetBuyStopLimitOrderMap()).size(), 0u);
	ASSERT_EQ(Flatten(market->GetBuyLimitOrderMap()).size(), 0u);

	// Check wallet
	auto& walletManager = tradingEngine.GetWalletManager();
	ASSERT_EQ(walletManager.GetWallet(coinPair.GetCoinId())->GetAddress(7)->GetInOrder(),
	Units::ExToIn(0.0));
	ASSERT_EQ(walletManager.GetWallet(coinPair.GetBaseId())->GetAddress(6)->GetInOrder(),
	Units::ExToIn(0.0));
	CompareOtherMarket(tradingEngine.GetMarketManager(), tradingEngine.GetWalletManager());
}

TEST_F(TradingEngineProcessing, ClearAllEveryonesOpenOrders) {
	Message message;
	message.messageType = MessageType::ClearAllEveryonesOpenOrders;
	auto outputMessages = tradingEngine.Process(message);

	auto market = tradingEngine.GetMarketManager().GetMarket(CreateCoinPair());
	ASSERT_EQ(Flatten(market->GetSellStopLimitOrderMap()).size(), 0u);
	ASSERT_EQ(Flatten(market->GetSellLimitOrderMap()).size(), 0u);
	ASSERT_EQ(Flatten(market->GetBuyStopLimitOrderMap()).size(), 0u);
	ASSERT_EQ(Flatten(market->GetBuyLimitOrderMap()).size(), 0u);

	auto otherMarket = tradingEngine.GetMarketManager().GetMarket(CreateAnotherCoinPair());
	ASSERT_EQ(Flatten(otherMarket->GetSellStopLimitOrderMap()).size(), 0u);
	ASSERT_EQ(Flatten(otherMarket->GetSellLimitOrderMap()).size(), 0u);
	ASSERT_EQ(Flatten(otherMarket->GetBuyStopLimitOrderMap()).size(), 0u);
	ASSERT_EQ(Flatten(otherMarket->GetBuyLimitOrderMap()).size(), 0u);

	auto& walletManager = tradingEngine.GetWalletManager();
	const auto& coinPair = CreateCoinPair();
	ASSERT_EQ(walletManager.GetWallet(coinPair.GetCoinId())->GetAddress(7)->GetInOrder(),
	Units::ExToIn(0.0));
	ASSERT_EQ(walletManager.GetWallet(coinPair.GetBaseId())->GetAddress(6)->GetInOrder(),
	Units::ExToIn(0.0));

	const auto& anotherCoinPair = CreateAnotherCoinPair();
	ASSERT_EQ(walletManager.GetWallet(anotherCoinPair.GetCoinId())->GetAddress(7)->GetInOrder(),
	Units::ExToIn(0.0));
	ASSERT_EQ(walletManager.GetWallet(anotherCoinPair.GetBaseId())->GetAddress(6)->GetInOrder(),
	Units::ExToIn(0.0));
}

TEST_F(TradingEngineProcessing, Deposit) {
	Message message;
	message.messageType = MessageType::Deposit;
	message.coinId = 3;
	message.userId = 6;
	message.amount = 1000;
	message.isRevertedDeposit = false;

	auto wallet = tradingEngine.GetWalletManager().GetWallet(message.coinId);
	auto address = wallet->GetAddress(message.userId);
	auto beforeBalance = address->GetTotalBalance();
	auto outputMessages = tradingEngine.Process(message);
	ASSERT_EQ(outputMessages.size(), 1u);
	ASSERT_EQ(outputMessages.front().errorCode, 0);
	ASSERT_EQ(message, outputMessages.front());
	auto afterBalance = address->GetTotalBalance();
	ASSERT_EQ(beforeBalance + message.amount, afterBalance);
	CompareOtherMarket(tradingEngine.GetMarketManager(), tradingEngine.GetWalletManager());
}

TEST_F(TradingEngineProcessing, Withdraw) {
	Message message;
	message.messageType = MessageType::Withdraw;
	message.coinId = 3;
	message.userId = 6;
	message.amount = 1000;

	auto wallet = tradingEngine.GetWalletManager().GetWallet(message.coinId);
	auto address = wallet->GetAddress(message.userId);
	auto beforeBalance = address->GetTotalBalance();
	auto outputMessages = tradingEngine.Process(message);
	ASSERT_EQ(outputMessages.size(), 1u);
	ASSERT_EQ(outputMessages.front().errorCode, 0);
	ASSERT_EQ(message, outputMessages.front());
	auto afterBalance = address->GetTotalBalance();
	ASSERT_EQ(beforeBalance - message.amount, afterBalance);
	CompareOtherMarket(tradingEngine.GetMarketManager(), tradingEngine.GetWalletManager());
}

TEST_F(TradingEngineProcessing, NewCoin) {
	Message message;
	message.messageType = MessageType::NewCoin;
	message.coinId = 10;
	auto outputMessages = tradingEngine.Process(message);

	ASSERT_EQ(outputMessages.size(), 1u);
	ASSERT_EQ(outputMessages.front().errorCode, 0);
	ASSERT_EQ(message, outputMessages.front());
	ASSERT_NO_THROW(tradingEngine.GetWalletManager().GetWallet(message.coinId));
	CompareOtherMarket(tradingEngine.GetMarketManager(), tradingEngine.GetWalletManager());
}

TEST_F(TradingEngineProcessing, NewMarket) {
	Message message;
	message.messageType = MessageType::NewMarket;
	message.coinId = 1;
	message.baseId = 3;
	message.feePercentage = 0.4;
	message.maxNumLimitOpenOrders = 100;
	message.maxNumStopLimitOpenOrders = 2;
	auto outputMessages = tradingEngine.Process(message);

	ASSERT_EQ(outputMessages.size(), 1u);
	ASSERT_EQ(outputMessages.front().errorCode, 0);
	ASSERT_EQ(message, outputMessages.front());
	ASSERT_NO_THROW(tradingEngine.GetMarketManager().GetMarket({ 1, 3 }));
	CompareOtherMarket(tradingEngine.GetMarketManager(), tradingEngine.GetWalletManager());
}

TEST_F(TradingEngineProcessing, GetAvailable) {
	Message message;
	message.messageType = MessageType::GetAvailable;
	message.coinId = 1;
	message.userId = BuyUserId();
	auto outputMessages = tradingEngine.Process(message);
	ASSERT_EQ(outputMessages.size(), 1u);
	ASSERT_EQ(outputMessages.front().errorCode, 0);
	ASSERT_EQ(outputMessages.front().amount, GetAvailable());
}

TEST_F(TradingEngineProcessing, GetAmount) {
	Message message;
	message.messageType = MessageType::GetAmount;
	message.coinId = 1;
	message.userId = 6;
	auto outputMessages = tradingEngine.Process(message);
	ASSERT_EQ(outputMessages.size(), 1u);
	ASSERT_EQ(outputMessages.front().errorCode, 0);
	ASSERT_EQ(outputMessages.front().amount, GetTotalBalance());
}

TEST_F(TradingEngineProcessing, SetInOrder) {
	Message message;
	message.messageType = MessageType::SetInOrder;
	message.coinId = 3;
	message.userId = 7;
	message.amount = Units::ExToIn(0.1);
	auto outputMessages = tradingEngine.Process(message);
	ASSERT_EQ(outputMessages.size(), 0u);
	auto wallet = tradingEngine.GetWalletManager().GetWallet(message.coinId);
	auto inOrder = wallet->GetAddress(message.userId)->GetInOrder();
	ASSERT_EQ(inOrder, message.amount);
}

TEST_F(TradingEngineProcessing, GetInOrder) {
	Message message;
	message.messageType = MessageType::GetInOrder;
	message.coinId = 1;
	message.userId = 6;
	auto outputMessages = tradingEngine.Process(message);
	ASSERT_EQ(outputMessages.size(), 1u);
	ASSERT_EQ(outputMessages.front().errorCode, 0);
	ASSERT_EQ(outputMessages.front().amount, GetInOrder());
}

TEST_F(TradingEngineProcessing, GetTotal) {
	Message message;
	message.messageType = MessageType::GetTotal;
	message.coinId = 1;
	auto outputMessages = tradingEngine.Process(message);
	ASSERT_EQ(outputMessages.size(), 1u);
	ASSERT_EQ(outputMessages.front().errorCode, 0);
	ASSERT_EQ(outputMessages.front().amount, GetCoinTotal());
}

TEST_F(TradingEngineProcessing, SetFeePercentage) {
	Message message;
	message.messageType = MessageType::SetFeePercentage;
	message.feePercentage = 0.2;
	auto outputMessages = tradingEngine.Process(message);
	ASSERT_EQ(outputMessages.size(), 0u);
	auto fee = tradingEngine.GetMarketManager().GetMarket(CreateCoinPair())->GetConfig().feeDivision;
	auto otherFee = Fee::ConvertToDivisibleFee(message.feePercentage);
	ASSERT_EQ(fee, otherFee);
}

TEST_F(TradingEngineProcessing, SetMaxNumLimitOpenOrders) {
	Message message;
	message.messageType = MessageType::SetMaxNumLimitOpenOrders;
	message.maxNumLimitOpenOrders = 500;
	auto outputMessages = tradingEngine.Process(message);
	ASSERT_EQ(outputMessages.size(), 0u);
	const auto& config = tradingEngine.GetMarketManager().GetMarket(CreateCoinPair())->GetConfig();
	ASSERT_EQ(config.maxNumLimitOpenOrders, message.maxNumLimitOpenOrders);
}

TEST_F(TradingEngineProcessing, SetMaxNumStopLimitOpenOrders) {
	Message message;
	message.messageType = MessageType::SetMaxNumStopLimitOpenOrders;
	message.maxNumStopLimitOpenOrders = 100;
	auto outputMessages = tradingEngine.Process(message);
	ASSERT_EQ(outputMessages.size(), 0u);
	const auto& config = tradingEngine.GetMarketManager().GetMarket(CreateCoinPair())->GetConfig();
	ASSERT_EQ(config.maxNumStopLimitOpenOrders, message.maxNumStopLimitOpenOrders);
}
