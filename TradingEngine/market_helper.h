#pragma once

#include "Orders/StopLimitOrder.h"
#include "PlatformSpecific/allocator_constants.h"
#include "PoolAlloc.h"
#include "SharedPoolAllocator.h"

#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <unordered_map>
#include <vector>

class IWallet;

struct MarketConfig {
	int32_t feeDivision;
	int32_t maxNumLimitOpenOrders;
	int32_t maxNumStopLimitOpenOrders;

	bool operator==(const MarketConfig& config) const {
		return (feeDivision == config.feeDivision
		&& maxNumLimitOpenOrders == config.maxNumLimitOpenOrders
		&& maxNumStopLimitOpenOrders == config.maxNumStopLimitOpenOrders);
	}
};

struct MarketWallets {
	IWallet* coinWallet;
	IWallet* baseWallet;
};

template <class Order, class Alloc>
using BaseOrders = std::deque<Order, Alloc>;

template <class Order>
using Orders = BaseOrders<Order, SharedPoolAllocator<Order, ps::GetOrderDequeSize<Order>(), 64>>;

template <class Order, class Sort, class Alloc = PoolAlloc<std::pair<const int64_t, Orders<Order>>, 1024>>
using OrderMap = std::map<int64_t, Orders<Order>, Sort, Alloc>;

template <class Sort>
using LimitOrderMap = OrderMap<LimitOrder, Sort>;
using BuyLimitOrderMap = LimitOrderMap<std::greater<int64_t>>;
using SellLimitOrderMap = LimitOrderMap<std::less<int64_t>>;

template <class Sort>
using StopLimitOrderMap = OrderMap<StopLimitOrder, Sort>;
using BuyStopLimitOrderMap = StopLimitOrderMap<std::less<int64_t>>;
using SellStopLimitOrderMap = StopLimitOrderMap<std::greater<int64_t>>;

struct PriceOrderId {
	int64_t price;
	int64_t orderId;

	bool operator==(const PriceOrderId& priceOrderId) const {
		return (price == priceOrderId.price && orderId == priceOrderId.orderId);
	}
};

struct UserOrders {
	// These should be sorted with the same as the respective map
	std::vector<PriceOrderId> buyLimitPrices;
	std::vector<PriceOrderId> sellLimitPrices;
	std::vector<PriceOrderId> buyStopLimitPrices;
	std::vector<PriceOrderId> sellStopLimitPrices;

	bool operator==(const UserOrders& userOrders) const {
		return (buyLimitPrices == userOrders.buyLimitPrices
		&& sellLimitPrices == userOrders.sellLimitPrices
		&& buyStopLimitPrices == userOrders.buyStopLimitPrices
		&& sellStopLimitPrices == userOrders.sellStopLimitPrices);
	}
};

using UserOrderMap = std::unordered_map<int32_t, UserOrders>;

template <class Comp>
struct CompareUserOrders {
	CompareUserOrders(const Comp& comp) :
	comp(comp) {
	}

	bool operator()(const PriceOrderId& priceOrderIdLhs, const PriceOrderId& priceId) {
		Comp comp;
		if (priceOrderIdLhs.price == priceId.price) {
			// Subsort by id
			return priceOrderIdLhs.orderId < priceId.orderId;
		}

		return comp(priceOrderIdLhs.price, priceId.price);
	}

private:
	Comp comp;
};

// This takes an order map and, creates an appropriate collection
// containing all sorted values from the map
template <class Order, class Sort>
std::deque<Order> Flatten(const OrderMap<Order, Sort>& orderMap) {
	std::deque<Order> flattenedOrders;

	for (auto& pair : orderMap) {
		const auto& orders = pair.second;

		flattenedOrders.insert(flattenedOrders.end(), orders.begin(), orders.end());
	}

	return flattenedOrders;
}
