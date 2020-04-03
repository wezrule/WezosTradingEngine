#pragma once

#include "MarketManager.h"
#include "Message.h"
#include "Orders/MarketOrder.h"
#include "Orders/OrderContainer.h"
#include "Orders/StopLimitOrder.h"
#include "WalletManager.h"
#include "serializer_defines.h"

#include <vector>

struct MarketWallets;

SERIALIZE_HEADER(TradingEngine)

class TradingEngine {
public:
	SERIALIZE_FRIEND(TradingEngine)

	TradingEngine() = default; // For serializing
	std::vector<Message> Process(const Message& message);
	bool operator==(const TradingEngine& tradingEngine) const;

	// Just for tests...
	MarketManager& GetMarketManager() { return marketManager; }
	WalletManager& GetWalletManager() { return walletManager; }

private:
	MarketManager marketManager;
	WalletManager walletManager;

	template <class T>
	OrderContainer<T> CreateOrder(const Message& message);

	MarketWallets GetMarketWallets(const Message& message);

	template <typename T>
	std::vector<Message> ProcessOrder(const Message& message);

	template <typename Order>
	void CancelOrder(const Message& message);
};

namespace boost::serialization {

template <class Archive>
void serialize(Archive& ar, TradingEngine& tradingEngine, const unsigned int version) {
	ar& tradingEngine.marketManager;
	ar& tradingEngine.walletManager;
}
}

template <>
OrderContainer<LimitOrder> TradingEngine::CreateOrder(const Message& message);

template <>
OrderContainer<StopLimitOrder> TradingEngine::CreateOrder(const Message& message);

template <>
OrderContainer<MarketOrder> TradingEngine::CreateOrder(const Message& message);
