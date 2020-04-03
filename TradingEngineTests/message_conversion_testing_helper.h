#include "MessageConversion.h"

#include <TradingEngine/Address.h>
#include <TradingEngine/CoinPair.h>
#include <TradingEngine/Listener/Listener.h>
#include <TradingEngine/Market.h>
#include <TradingEngine/MarketManager.h>
#include <TradingEngine/Message.h>
#include <TradingEngine/Units.h>
#include <TradingEngine/Wallet.h>
#include <TradingEngine/WalletManager.h>
#include <TradingEngine/market_helper.h>
#include <memory>

inline int64_t GetTotalBalance() {
	return Units::ExToIn(1500.0);
}

inline int64_t GetCoinTotal() {
	return 2 * GetTotalBalance();
}

inline int64_t GetTotal() {
	return GetTotalBalance() * 2;
}

inline CoinPair CreateCoinPair() {
	return { 3, 1 };
}

inline CoinPair CreateAnotherCoinPair() {
	return { 4, 2 };
}

inline int32_t BuyUserId() {
	return 6;
}

inline int32_t SellUserId() {
	return 7;
}

inline int64_t GetOrderAmount() {
	return Units::ExToIn(200.0);
}

inline int64_t GetInOrder() {
	return (4 * GetOrderAmount());
}

inline int64_t GetAvailable() {
	return GetTotalBalance() - GetInOrder();
}

static CoinPair GetCoinPair(int index) {
	return (index == 0) ? CreateCoinPair() : CreateAnotherCoinPair();
}

inline Address CreateAddress(int32_t userId) {
	Address address{ userId };
	address.SetTotalBalance(GetTotalBalance());
	return address;
}

inline Wallet CreateWallet(int index) {
	Wallet wallet{ GetCoinPair(index).GetCoinId() };
	wallet.AddAddress(CreateAddress(BuyUserId()));
	wallet.AddAddress(CreateAddress(SellUserId()));
	return wallet;
}

inline Wallet CreateBaseWallet(int index) {
	Wallet wallet{ GetCoinPair(index).GetBaseId() };
	wallet.AddAddress(CreateAddress(BuyUserId()));
	wallet.AddAddress(CreateAddress(SellUserId()));
	return wallet;
}

inline LimitOrder CreateLimitOrder(int32_t userId) {
	return { userId, GetOrderAmount(), 0 };
}

inline StopLimitOrder CreateStopLimitOrder(int32_t userId, int64_t stopPrice) {
	return { userId, GetOrderAmount(), 0, stopPrice };
}

inline MarketConfig CreateMarketConfig() {
	return { 1000, 500, 100 };
}

template <OrderAction Side>
OrderContainer<LimitOrder> GetLimitOrder(int index) {
	if constexpr (Side == OrderAction::Buy) {
		if (index == 0) {
			return { { CreateLimitOrder(BuyUserId()) }, Units::ExToIn(0.5) };
		} else {
			return { { CreateLimitOrder(BuyUserId()) }, Units::ExToIn(0.4) };
		}
	} else {
		if (index == 0) {
			return { { CreateLimitOrder(SellUserId()) }, Units::ExToIn(0.6) };
		} else {
			return { { CreateLimitOrder(SellUserId()) }, Units::ExToIn(0.7) };
		}
	}
}

template <OrderAction Side>
OrderContainer<StopLimitOrder> GetStopLimitOrder(int index) {
	if constexpr (Side == OrderAction::Buy) {
		if (index == 0) {
			return { CreateStopLimitOrder(BuyUserId(), Units::ExToIn(0.6)), Units::ExToIn(0.6) };
		} else {
			return { CreateStopLimitOrder(BuyUserId(), Units::ExToIn(0.8)), Units::ExToIn(0.7) };
		}
	} else {
		if (index == 0) {
			return { CreateStopLimitOrder(SellUserId(), Units::ExToIn(0.5)), Units::ExToIn(0.5) };
		} else {
			return { CreateStopLimitOrder(SellUserId(), Units::ExToIn(0.3)), Units::ExToIn(0.4) };
		}
	}
}

inline Market CreateMarket(MarketWallets* marketWallets, int index) {
	Market market(std::make_unique<Listener>(), GetCoinPair(index), CreateMarketConfig());

	// This market looks as follow:
	// Where it is: |orderId|userId|price[|stopPrice]|amount|
	/*
  4|7|0.7|200|   8|7|0.4|0.3|200|
  3|7|0.6|200|   7|7|0.5|0.5|200|
  ___________      _______________

  1|6|0.5|200|   5|6|0.6|0.6|200|
  2|6|0.4|200|   6|6|0.7|0.8|200|
*/

	market.NewProcess<OrderAction::Buy>(GetLimitOrder<OrderAction::Buy>(0), marketWallets);
	market.NewProcess<OrderAction::Buy>(GetLimitOrder<OrderAction::Buy>(1), marketWallets);

	market.NewProcess<OrderAction::Sell>(GetLimitOrder<OrderAction::Sell>(0), marketWallets);
	market.NewProcess<OrderAction::Sell>(GetLimitOrder<OrderAction::Sell>(1), marketWallets);

	market.NewProcess<OrderAction::Buy>(GetStopLimitOrder<OrderAction::Buy>(0), marketWallets);
	market.NewProcess<OrderAction::Buy>(GetStopLimitOrder<OrderAction::Buy>(1), marketWallets);

	market.NewProcess<OrderAction::Sell>(GetStopLimitOrder<OrderAction::Sell>(0), marketWallets);
	market.NewProcess<OrderAction::Sell>(GetStopLimitOrder<OrderAction::Sell>(1), marketWallets);

	market.GetListener().ClearOperations();
	return market;
}

inline MarketManager CreateMarketManager(MarketWallets* marketWallets, MarketWallets* marketWallets1) {
	MarketManager marketManager;
	marketManager.AddMarket(CreateMarket(marketWallets, 0));
	marketManager.AddMarket(CreateMarket(marketWallets1, 1));
	return marketManager;
}

inline WalletManager CreateWalletManager() {
	WalletManager walletManager;
	walletManager.AddWallet(CreateWallet(0));
	walletManager.AddWallet(CreateBaseWallet(0));
	walletManager.AddWallet(CreateWallet(1));
	walletManager.AddWallet(CreateBaseWallet(1));
	return walletManager;
}

template <class T>
void AddMessages(T t, std::vector<Message>* messages) {
	std::vector<Message> newMessages = ToMessages(t);
	messages->insert(messages->end(), newMessages.begin(), newMessages.end());
}

inline std::vector<Message> CreateSimpleMessages() {
	std::vector<Message> messages;

	AddMessages(CreateWalletManager(), &messages);

	// Add market
	Wallet coinWallet = CreateWallet(0);
	Wallet baseWallet = CreateBaseWallet(0);
	MarketWallets marketWallets{ &coinWallet, &baseWallet };

	Wallet coinWallet1 = CreateWallet(1);
	Wallet baseWallet1 = CreateBaseWallet(1);
	MarketWallets marketWallets1{ &coinWallet1, &baseWallet1 };

	AddMessages(CreateMarketManager(&marketWallets, &marketWallets1), &messages);

	return messages;
}
