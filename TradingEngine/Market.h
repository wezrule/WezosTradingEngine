#pragma once

#include "Address.h"
#include "CoinPair.h"
#include "Listener/IListener.h"
#include "Orders/MarketOrder.h"
#include "Orders/OrderAction.h"
#include "Orders/OrderContainer.h"
#include "Orders/StopLimitOrder.h"
#include "Simulator.h"
#include "market_helper.h"
#include "serializer_defines.h"

#include <cstdint>
#include <deque>
#include <memory>
#include <tuple>
#include <vector>

class IListener;
class IWallet;

SERIALIZE_HEADER(Market)

class Market {
public:
	SERIALIZE_FRIEND(Market)

	Market() = default; // For serializing
	Market(std::unique_ptr<IListener>&& listener, const CoinPair& coinPair,
	const MarketConfig& config);
	Market(std::unique_ptr<IListener>&& listener, const CoinPair& coinPair, double feePercentage,
	int32_t maxNumLimitOpenOrders, int32_t maxNumStopLimitOpenOrders);
	Market(const Market& market);
	Market& operator=(const Market& market);
	Market& operator=(Market&& other) noexcept = default;
	Market(Market&& other) noexcept = default;

	// This is the main entrance point.
	template <OrderAction Side, class T>
	void NewProcess(const OrderContainer<T>& orderContainer, MarketWallets* coinWallets);

	const BuyLimitOrderMap& GetBuyLimitOrderMap() const;
	const SellLimitOrderMap& GetSellLimitOrderMap() const;
	const BuyStopLimitOrderMap& GetBuyStopLimitOrderMap() const;
	const SellStopLimitOrderMap& GetSellStopLimitOrderMap() const;

	void SetListener(std::unique_ptr<IListener>&& listener);

	const CoinPair& GetCoinPair() const;
	bool operator==(const Market& market) const;

	void SetMaxOrderId(int64_t id);
	void SetMaxTradeId(int64_t id);

	template <OrderAction Side, class Order>
	void CancelOrder(int64_t id, int64_t price, MarketWallets* marketWallets);

	void CancelAll();
	std::vector<int64_t> CancelAll(int32_t userId);

	// For testing...
	template <OrderAction Side, class Order>
	const std::vector<PriceOrderId>& GetUserOrderCache(int32_t userId);

	void SetFeePercentage(double fee);
	void SetMaxNumLimitOpenOrders(int32_t numOpenOrders);
	void SetMaxNumStopLimitOpenOrders(int32_t numOpenOrders);

	IListener& GetListener() const;
	const MarketConfig& GetConfig() const;
	UserOrders& GetUserOrders(int32_t userId);
	const UserOrderMap& GetUserOrderMap() const;

	template <OrderAction Side, class Order>
	void ForceAddOrder(const OrderContainer<Order>& orderContainer);

private:
	std::unique_ptr<IListener> listener;

	// This uniquely identifies the market.
	CoinPair coinPair;
	BuyLimitOrderMap buyLimitOrderMap;
	SellLimitOrderMap sellLimitOrderMap;

	BuyStopLimitOrderMap buyStopLimitOrderMap;
	SellStopLimitOrderMap sellStopLimitOrderMap;

	// This is a map of user ids with a collection of all the orders they have open
	UserOrderMap userOrderMap;

	int64_t currentOrderId = 1;
	int64_t currentTradeId = 1;

	MarketConfig config;

	// This contains the changes that will be committed later after processing successfully.
	// It is the only variable which should be modified inside the const methods.
	mutable Simulator simulator;

	void PreProcess() const;

	template <OrderAction Side, class T>
	void Process(const OrderContainer<T>& orderContainer, MarketWallets* marketWallets) const;

	template <OrderAction Side, class Order>
	void PostProcess(const OrderContainer<Order>& orderContainer, MarketWallets* marketWallets);

	template <OrderAction Side, class T>
	void CommitChanges(const OrderContainer<T>& orderContainer, MarketWallets* marketWallets);

	bool SimulateMarketBuyOrder(const MarketOrder& marketBuyOrder, int64_t availableBalance) const;

	template <OrderAction Side, class T, class T1>
	void CommitChangesHelper(int64_t orderRemaining,
	LimitOrderMap<T>& insertedLimitOrderMap, LimitOrderMap<T1>& updatedLimitOrderMap,
	StopLimitOrderMap<T1>& stopOrderMap, std::vector<Address>::iterator origAddress,
	MarketWallets* marketWallets);

	template <OrderAction Side, class T>
	void ThrowIfInsufficientFunds(const OrderContainer<T>& order, int64_t availableBalance) const = delete;

	template <OrderAction Side, class Sort>
	void ProcessMarketOrder(const OrderContainer<MarketOrder>& orderContainer,
	const LimitOrderMap<Sort>& limitOrders, const StopLimitOrderMap<Sort>& stopLimitOrders,
	MarketWallets* marketWallets) const;

	template <OrderAction Side, class Sort>
	void ProcessLimitOrder(const OrderContainer<LimitOrder>& inOrderContainer,
	const LimitOrderMap<Sort>& limitOrders, const StopLimitOrderMap<Sort>& stopLimitOrders,
	MarketWallets* marketWallets) const;

	template <OrderAction Side, class Sort>
	void ProcessStopLimitOrder(const OrderContainer<StopLimitOrder>& orderContainer,
	const LimitOrderMap<Sort>& limitOrders,
	const StopLimitOrderMap<Sort>& stopLimitOrders) const;

	template <OrderAction Side, class T>
	void ValidateFunds(MarketWallets* marketWallets, const OrderContainer<T>& orderContainer) const;

	template <OrderAction Side>
	std::tuple<int64_t, int64_t, int32_t, int32_t> ConsumeHelper(int64_t origOrderId,
	int64_t orderId, int64_t origUserId, int32_t userId) const;

	Fee CalculateFees(int64_t amount, int64_t price) const;

	template <OrderAction Side, class T, class Sort, class Sort1>
	void ConsumeOrderBook(OrderContainer<T>* orderContainer,
	int64_t* lastTradePrice,
	const LimitOrderMap<Sort>& limitOrderMap) const;

	template <OrderAction Side, class T>
	bool Consume(OrderContainer<T>* orderContainer, int64_t price, int64_t* lastTradePrice,
	typename std::deque<LimitOrder>::const_iterator start,
	typename std::deque<LimitOrder>::const_iterator end) const;

	template <OrderAction Side, class Sort>
	void KickOffStopOrders(const StopLimitOrderMap<Sort>& stopLimitOrders,
	int64_t lastTradePrice, MarketWallets* marketWallets) const;

	template <OrderAction Side, class Sort, class StopSort>
	void ProcessStopOrders(const StopLimitOrderMap<Sort>& stopLimitOrderMap,
	int64_t lastTradePrice,
	MarketWallets* marketWallets) const;

	template <OrderAction Side, class T>
	constexpr void NewOpenOrder(const OrderContainer<T>& order) const;

	template <OrderAction Side, class Comp, class Order>
	void RemoveOrders(int32_t userId, int64_t price, int64_t stopOrderId);

	LimitOrder ConvertToLimitOrder(const StopLimitOrder& stopLimitOrder) const;

	template <OrderAction Side, class Order>
	std::vector<PriceOrderId>& GetUserOrderCacheHelper(UserOrders* userOrders);

	template <OrderAction Side, class Order, class Comp>
	void AddToUserCache(int32_t userId, int64_t price, int64_t orderId);

	template <OrderAction Side, class Order>
	void ValidateSameUserOrder(const OrderContainer<Order>& orderContainer);

	int32_t NumLimitOpenOrders(int32_t userId) const;
	int32_t NumStopLimitOpenOrders(int32_t userId) const;

	template <OrderAction Side, class Order, class Comp>
	void ForceAdd(OrderMap<Order, Comp>& orderMap, const OrderContainer<Order>& orderContainer);

	template <OrderAction Side, class Order, class Comp>
	void CancelHelper(OrderMap<Order, Comp>& orderMap, int64_t id, int64_t price, MarketWallets* marketWallets);

	template <OrderAction Side, class Order, typename Comp>
	void CancelOrders(OrderMap<Order, Comp>& orderMap, std::vector<PriceOrderId>& priceOrderIds);
};
