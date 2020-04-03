#include "Market.h"

#include "Error.h"
#include "Fee.h"
#include "IWallet.h"
#include "Listener/IListener.h"
#include "SimulatorTrade.h"
#include "Units.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <unordered_map>
#include <utility>

Market::Market(std::unique_ptr<IListener>&& listener, const CoinPair& coinPair,
const MarketConfig& config) :
listener(std::move(listener)),
coinPair(coinPair),
config(config) {
	if (coinPair.GetBaseId() == coinPair.GetCoinId()) {
		throw Error(Error::Type::CoinIdsSame, "Base and Coin Id are the same...");
	}
}

Market::Market(std::unique_ptr<IListener>&& listener, const CoinPair& coinPair,
double feePercentage, int32_t maxNumLimitOpenOrders, int32_t maxNumStopLimitOpenOrders) :
Market(std::move(listener), coinPair, { Fee::ConvertToDivisibleFee(feePercentage), maxNumLimitOpenOrders, maxNumStopLimitOpenOrders }) {
}

Market::Market(const Market& market) {
	listener = market.listener->Clone();
	coinPair = market.coinPair;

	buyLimitOrderMap = market.buyLimitOrderMap;
	sellLimitOrderMap = market.sellLimitOrderMap;

	buyStopLimitOrderMap = market.buyStopLimitOrderMap;
	sellStopLimitOrderMap = market.sellStopLimitOrderMap;

	// This is a map of user ids with a collection of all the orders they have open
	userOrderMap = market.userOrderMap;

	currentOrderId = market.currentOrderId;
	currentTradeId = market.currentTradeId;

	config = market.config;

	// This contains the changes that will be committed later after processing successfully
	simulator = market.simulator;
}

Market& Market::operator=(const Market& other) {
	Market market(other); // Reuse copy constructor
	*this = std::move(market); // Reuse move constructor
	return *this;
}

// Clean set up before any processing should be done
void Market::PreProcess() const {
	simulator.SetCurrentOrderId(currentOrderId);
	simulator.SetCurrentTradeId(currentTradeId);
}

// This is the main entry point.
template <OrderAction Side, class Order>
void Market::NewProcess(const OrderContainer<Order>& orderContainer,
MarketWallets* marketWallets) {
	PreProcess();

	ValidateFunds<Side>(marketWallets, orderContainer);

	ValidateSameUserOrder<Side>(orderContainer);

	const_cast<Order&>(orderContainer.order).SetId(simulator.GetCurrentOrderId());

	try {
		Process<Side>(orderContainer, marketWallets);
		simulator.IncrementOrderId();
	} catch (...) {
		simulator.Clear();
		throw; // Rethrow exception
	}

	PostProcess<Side>(orderContainer, marketWallets);
}

// Actually make the changes from the simulator
template <OrderAction Side, class Order>
void Market::PostProcess(const OrderContainer<Order>& orderContainer,
MarketWallets* marketWallets) {
	currentOrderId = simulator.GetCurrentOrderId();
	currentTradeId = simulator.GetCurrentTradeId();
	CommitChanges<Side>(orderContainer, marketWallets);
	simulator.Clear();
}

void Market::SetListener(std::unique_ptr<IListener>&& listener) {
	this->listener = std::move(listener);
}

// Check that the user has enough funds to make the order.
template <OrderAction Side, class T>
void Market::ValidateFunds(MarketWallets* marketWallets,
const OrderContainer<T>& orderContainer) const {
	if constexpr (Side == OrderAction::Buy) {
		auto address = marketWallets->baseWallet->GetAddress(orderContainer.order.GetUserId());
		auto availableBalance = address->GetAvailableBalance();
		ThrowIfInsufficientFunds<OrderAction::Buy>(orderContainer, availableBalance);
	} else {
		auto availableBalance = marketWallets->coinWallet->GetAddress(orderContainer.order.GetUserId())->GetAvailableBalance();
		if (availableBalance < orderContainer.order.GetRemaining()) {
			throw Error(Error::Type::InsufficientFunds, "User doesn't have enough coins to make this sell order");
		}
	}
}

// Check that. This only needs to be done once on the initial order, rather than in
// "Process*Order". Throws if order is not valid
template <OrderAction Side, class Order>
void Market::ValidateSameUserOrder(const OrderContainer<Order>& orderContainer) {
	if constexpr (IsStopLimitOrder_v<Order>) {
		// Check that this user does not have any existing limit orders in the other order book,
		// which may cause this stop-limit order to execute that one (i.e trade with yourself).
		auto limitOrderPrice = orderContainer.order.GetActualPrice();
		auto it = userOrderMap.find(orderContainer.order.GetUserId());
		if (it == userOrderMap.end()) {
			return;
		}

		// Found
		const auto& userOrders = it->second;
		if constexpr (Side == OrderAction::Buy) {
			Check(sellLimitOrderMap.key_comp(), userOrders.sellLimitPrices, limitOrderPrice);
		} else {
			Check(buyLimitOrderMap.key_comp(), userOrders.buyLimitPrices, limitOrderPrice);
		}
	} else if constexpr (IsLimitOrder_v<Order>) {
		// Check that this user does not have any existing limit orders in the other order book,
		// which may cause this stop-limit order to execute that one (i.e trade with yourself).
		auto limitOrderPrice = orderContainer.GetPrice();
		auto it = userOrderMap.find(orderContainer.order.GetUserId());
		if (it == userOrderMap.end()) {
			return;
		}

		// Found
		const auto& userOrders = it->second;
		if constexpr (Side == OrderAction::Buy) {
			Check(sellStopLimitOrderMap.key_comp(), userOrders.sellStopLimitPrices, limitOrderPrice);
		} else {
			Check(buyStopLimitOrderMap.key_comp(), userOrders.buyStopLimitPrices, limitOrderPrice);
		}
	}
}

// OrderMap seems to only be here for "Comp" deduction
template <class Comp>
void Check(Comp comp, const std::vector<PriceOrderId>& orders,
int64_t price) {
	auto it = std::lower_bound(orders.begin(), orders.end(), price,
	[comp](const auto& priceOrderIdLhs, int64_t price) {
		return comp(priceOrderIdLhs.price, price);
	});

	if (it != orders.end()) {
		// Should not be allowed
		throw Error(Error::Type::IncompatibleOrders,
		"Cannot make this limit/stop-limit order, if there is already a stop-limit/limit "
		"order on the other book which may result in the user making an order with himself "
		"in the future");
	}
}

template <>
void Market::Process<OrderAction::Buy>(const OrderContainer<MarketOrder>& orderContainer,
MarketWallets* marketWallets) const {
	ProcessMarketOrder<OrderAction::Buy>(orderContainer, sellLimitOrderMap,
	buyStopLimitOrderMap, marketWallets);
}

template <>
void Market::Process<OrderAction::Sell>(const OrderContainer<MarketOrder>& orderContainer,
MarketWallets* marketWallets) const {
	ProcessMarketOrder<OrderAction::Sell>(orderContainer, buyLimitOrderMap,
	sellStopLimitOrderMap, marketWallets);
}

template <>
void Market::Process<OrderAction::Buy>(const OrderContainer<LimitOrder>& orderContainer,
MarketWallets* marketWallets) const {
	ProcessLimitOrder<OrderAction::Buy>(orderContainer, sellLimitOrderMap, buyStopLimitOrderMap,
	marketWallets);
}

template <>
void Market::Process<OrderAction::Sell>(const OrderContainer<LimitOrder>& orderContainer,
MarketWallets* marketWallets) const {
	ProcessLimitOrder<OrderAction::Sell>(orderContainer, buyLimitOrderMap,
	sellStopLimitOrderMap, marketWallets);
}

template <>
void Market::Process<OrderAction::Buy>(const OrderContainer<StopLimitOrder>& orderContainer,
MarketWallets* marketWallets) const {
	ProcessStopLimitOrder<OrderAction::Buy>(orderContainer, sellLimitOrderMap, buyStopLimitOrderMap);
}

template <>
void Market::Process<OrderAction::Sell>(const OrderContainer<StopLimitOrder>& orderContainer,
MarketWallets* marketWallets) const {
	ProcessStopLimitOrder<OrderAction::Sell>(orderContainer, buyLimitOrderMap, sellStopLimitOrderMap);
}

template <OrderAction Side, class Comp>
void Market::ProcessMarketOrder(const OrderContainer<MarketOrder>& inOrderContainer,
const LimitOrderMap<Comp>& limitOrders,
const StopLimitOrderMap<Comp>& stopLimitOrders,
MarketWallets* marketWallets) const {
	int64_t lastTradePrice = -1;
	OrderContainer<MarketOrder> orderContainer = inOrderContainer;

	if constexpr (std::is_convertible_v<Comp, std::less<int64_t>>) {
		ConsumeOrderBook<Side, MarketOrder, Comp, std::less_equal<int64_t>>(&orderContainer,
		&lastTradePrice,
		limitOrders);
	} else {
		ConsumeOrderBook<Side, MarketOrder, Comp, std::greater_equal<int64_t>>(&orderContainer,
		&lastTradePrice,
		limitOrders);
	}

	// There isn't enough limit orders to process the market order.
	if (orderContainer.order.GetRemaining() != 0) {
		throw Error(Error::Type::MarketOrderUnfilled, "Market order cannot be fulfilled");
	}

	KickOffStopOrders<Side>(stopLimitOrders, lastTradePrice, marketWallets);
}

int32_t Market::NumLimitOpenOrders(int32_t userId) const {
	auto it = userOrderMap.find(userId);
	if (it == userOrderMap.end()) {
		return 0;
	}

	auto& userOpenOrders = it->second;
	return static_cast<int32_t>(userOpenOrders.buyLimitPrices.size() + userOpenOrders.sellLimitPrices.size());
}

int32_t Market::NumStopLimitOpenOrders(int32_t userId) const {
	auto it = userOrderMap.find(userId);
	if (it == userOrderMap.end()) {
		return 0;
	}

	auto& userOpenOrders = it->second;
	return static_cast<int32_t>(userOpenOrders.buyStopLimitPrices.size() + userOpenOrders.sellStopLimitPrices.size());
}

template <OrderAction Side, class Order>
constexpr void Market::NewOpenOrder(const OrderContainer<Order>& orderContainer) const {
	// Check the user hasn't reached the maximum number of allowed open orders
	if constexpr (IsLimitOrder_v<Order>) {
		if (NumLimitOpenOrders(orderContainer.order.GetUserId()) >= config.maxNumLimitOpenOrders) {
			throw Error(Error::Type::ReachedNumOpenOrders,
			"You have reached the maximum number of limit open orders allowed");
		}
	} else {
		static_assert(IsStopLimitOrder_v<Order>);
		if (NumLimitOpenOrders(orderContainer.order.GetUserId()) >= config.maxNumStopLimitOpenOrders) {
			throw Error(Error::Type::ReachedNumOpenOrders,
			"You have reached the maximum number of stop-limit open orders allowed");
		}
	}

	if constexpr (Side == OrderAction::Buy) {
		listener->NewOpenOrder(ConvertToListenerOrder(orderContainer), OrderAction::Buy);
	} else {
		listener->NewOpenOrder(ConvertToListenerOrder(orderContainer), OrderAction::Sell);
	}
}

template <OrderAction Side, class Comp>
void Market::ProcessLimitOrder(const OrderContainer<LimitOrder>& inOrderContainer,
const LimitOrderMap<Comp>& limitOrders,
const StopLimitOrderMap<Comp>& stopLimitOrders,
MarketWallets* marketWallets) const {
	// Check if any orders satisfy this limit order
	OrderContainer<LimitOrder> orderContainer = inOrderContainer;
	int64_t lastTradePrice = -1;
	if (!limitOrders.empty()) {
		if constexpr (std::is_convertible_v<Comp, std::less<int64_t>>) {
			ConsumeOrderBook<Side, LimitOrder, Comp, std::less_equal<int64_t>>(&orderContainer,
			&lastTradePrice,
			limitOrders);
		} else {
			ConsumeOrderBook<Side, LimitOrder, Comp, std::greater_equal<int64_t>>(&orderContainer,
			&lastTradePrice,
			limitOrders);
		}
	}

	if (orderContainer.order.GetRemaining() != 0) {
		NewOpenOrder<Side>(orderContainer);
		simulator.InsertLimitOrder(orderContainer.GetPrice(), orderContainer.order);
	}

	KickOffStopOrders<Side>(stopLimitOrders, lastTradePrice, marketWallets);
}

template <OrderAction Side, class Comp>
void Market::ProcessStopLimitOrder(const OrderContainer<StopLimitOrder>& orderContainer,
const LimitOrderMap<Comp>& limitOrders,
const StopLimitOrderMap<Comp>& stopLimitOrders) const {
	Comp comp;
	if (limitOrders.empty()) {
		throw Error(Error::Type::NoOrdersCannotPlaceStopLimit,
		"There are no orders so cannot place stop limit");
	} else if (comp(orderContainer.GetPrice(), limitOrders.begin()->first)) {
		throw Error(Error::Type::StopPriceTooHighLow,
		"Buy stop limit price is lower than the current ask order or"
		" sell stop price is higher than current bid order");
	} else {
		simulator.SetInsertedStopLimitOrder(orderContainer.price, orderContainer.order);
		NewOpenOrder<Side>(orderContainer);
	}
}

template <OrderAction Side, class Comp>
void Market::KickOffStopOrders(const StopLimitOrderMap<Comp>& stopLimitOrders,
int64_t lastTradePrice, MarketWallets* marketWallets) const {
	if (lastTradePrice != -1) {
		// Trade was done
		if constexpr (std::is_convertible_v<Comp, std::less<int64_t>>) {
			ProcessStopOrders<Side, Comp, std::less_equal<int64_t>>(stopLimitOrders, lastTradePrice,
			marketWallets);
		} else {
			ProcessStopOrders<Side, Comp, std::greater_equal<int64_t>>(stopLimitOrders,
			lastTradePrice, marketWallets);
		}
	}
}

template <OrderAction Side, class Comp, class StopComp>
void Market::ProcessStopOrders(const StopLimitOrderMap<Comp>& stopLimitOrderMap,
int64_t lastTradePrice, MarketWallets* marketWallets) const {
	LimitOrderMap<Comp> convertedStopToLimitOrderMap;

	// Firstly skip the number of stop limit orders to remove
	auto numTriggeredStopOrders = simulator.GetNumTriggeredStopOrders();

	// Initially skip the ones which have already been processed
	auto it = stopLimitOrderMap.cbegin();
	for (; it != stopLimitOrderMap.cend(); ++it) {
		// All stop orders of a certain price should be processed..
		if (numTriggeredStopOrders != 0) {
			numTriggeredStopOrders -= it->second.size();
		} else {
			break;
		}
	}

	std::vector<OrderContainer<LimitOrder>> convertedStopToLimitOrders;

	while (it != stopLimitOrderMap.end()) {
		// Should this price point be processed?
		auto price = it->first;
		StopComp comp;
		bool triggerStopOrder = comp(price, lastTradePrice);
		if (triggerStopOrder) {
			// Loop through all stop limit orders of this vector
			auto& stopLimitOrders = it->second;
			for (auto& stopLimitOrder : stopLimitOrders) {
				auto limitOrder = ConvertToLimitOrder(stopLimitOrder);
				convertedStopToLimitOrders.push_back({ limitOrder, stopLimitOrder.GetActualPrice() });
				simulator.IncrementNumTriggeredStopOrders();
			}
		} else {
			break;
		}
		++it;
	}

	auto triggeredTradeId = simulator.GetCurrentTradeId() - 1;
	for (auto& limitOrderContainer : convertedStopToLimitOrders) {
		listener->StopLimitTriggered(limitOrderContainer.order.GetId(), triggeredTradeId);
		Process<Side, LimitOrder>(limitOrderContainer, marketWallets);
	}
}

template <OrderAction Side, class T, class Comp, class Comp1>
void Market::ConsumeOrderBook(OrderContainer<T>* orderContainer, int64_t* lastTradePrice,
const LimitOrderMap<Comp>& limitOrderMap) const {
	auto& order = orderContainer->order;

	// Firstly skip the number of limit orders to remove
	auto numLimitOrdersToRemove = simulator.GetNumLimitOrdersToRemove();

	// Initially skip the ones which have already been processed
	auto it = limitOrderMap.cbegin();
	for (; it != limitOrderMap.cend(); ++it) {
		if (numLimitOrdersToRemove != 0) {
			auto count = static_cast<int>(it->second.size());
			if (numLimitOrdersToRemove > count) {
				numLimitOrdersToRemove -= count;
			} else {
				auto orderIter = it->second.begin();
				Consume<Side>(orderContainer, it->first, lastTradePrice, orderIter + numLimitOrdersToRemove, it->second.end());
				++it;
				break;
			}
		} else {
			break;
		}
	}

	// Now process the rest
	if (order.GetRemaining() != 0) {
		// Process the rest of the orders
		for (; it != limitOrderMap.cend(); ++it) {
			auto& [price, limitOrders] = *it;

			// Market Order price is 0, so don't execute the price comparison...
			if constexpr (!IsMarketOrder<T>::value) {
				Comp1 comp;
				if (!comp(price, orderContainer->GetPrice())) {
					break;
				}
			}

			// Will this order consume a price in the book?
			bool finishedOrder = Consume<Side>(orderContainer, price, lastTradePrice,
			limitOrders.begin(), limitOrders.end());
			if (finishedOrder) {
				break;
			}
		}
	}
}

template <>
std::tuple<int64_t, int64_t, int32_t, int32_t> Market::ConsumeHelper<OrderAction::Buy>(
int64_t origOrderId, int64_t orderId, int64_t origUserId, int32_t userId) const {
	return { origOrderId, orderId, origUserId, userId };
}

template <>
std::tuple<int64_t, int64_t, int32_t, int32_t> Market::ConsumeHelper<OrderAction::Sell>(
int64_t origOrderId, int64_t orderId, int64_t origUserId, int32_t userId) const {
	return { orderId, origOrderId, userId, origUserId };
}

template <OrderAction Side, class T>
bool Market::Consume(OrderContainer<T>* orderContainer, int64_t price, int64_t* lastTradePrice,
typename std::deque<LimitOrder>::const_iterator start,
typename std::deque<LimitOrder>::const_iterator end) const {
	auto& order = orderContainer->order;
	auto orderIter = start;
	while (orderIter != end) {
		if (orderIter->GetUserId() == order.GetUserId()) {
			throw Error(Error::Type::TradeSameUser, "Cannot trade your own order");
		}

		auto orderTotalCoins = orderIter->GetRemaining() - simulator.GetLastFill();
		*lastTradePrice = price; // TODO: Seems redundant
		auto orderRemaining = order.GetRemaining();

		auto [buyId, sellId, buyUserId, sellUserId] = ConsumeHelper<Side>(order.GetId(),
		orderIter->GetId(), order.GetUserId(), orderIter->GetUserId());

		if (orderTotalCoins > orderRemaining) {
			// Eat into it
			simulator.AddToLastFill(orderRemaining);
			order.AddToFill(orderRemaining);
			auto fees = CalculateFees(orderRemaining, price);
			listener->NewTrade(simulator.GetCurrentTradeId(), buyId, sellId, orderRemaining, price,
			fees);
			simulator.AddTrade(buyUserId, sellUserId, orderRemaining, price, fees);
			listener->PartialFill(orderIter->GetId(), orderRemaining);
			simulator.IncrementTradeId();

			// TODO, make this a template listener->NewFilledOrder<Side>(); ?
			if constexpr (Side == OrderAction::Buy) {
				listener->NewFilledOrder(ConvertToListenerOrder(*orderContainer), OrderAction::Buy);
			} else {
				listener->NewFilledOrder(ConvertToListenerOrder(*orderContainer), OrderAction::Sell);
			}

			return true;
		} else {
			// Consume the whole order
			simulator.IncrementNumLimitOrdersToRemove();
			simulator.SetLastFill(0);
			order.AddToFill(orderTotalCoins);
			auto fees = CalculateFees(orderTotalCoins, price);
			listener->NewTrade(simulator.GetCurrentTradeId(), buyId, sellId, orderTotalCoins, price,
			fees);
			simulator.AddTrade(buyUserId, sellUserId, orderTotalCoins, price, fees);
			listener->OrderFilled(orderIter->GetId());
			simulator.IncrementTradeId();

			// This might have consumed the rest of the needed orders
			if (order.GetRemaining() == 0) {
				listener->OrderFilled(order.GetId());
				return true;
			}

			++orderIter;
			continue;
		}

		throw Error(Error::Type::InternalIteratorInvalid, "Iterator won't be valid");
	}

	return false;
}

// If you want to market buy 1 REQ, you pay a fee after this, so end up with e.g 0.999
bool Market::SimulateMarketBuyOrder(const MarketOrder& marketBuyOrder,
int64_t availableBalance) const {
	auto order = marketBuyOrder;

	auto remainingBalance = availableBalance;

	for (const auto& [price, sellOrders] : sellLimitOrderMap) {
		for (const auto& sellOrder : sellOrders) {
			if (sellOrder.GetUserId() == marketBuyOrder.GetUserId()) {
				throw Error(Error::Type::TradeSameUser, "Cannot buy your own sell order");
			}

			if (sellOrder.GetRemaining() > order.GetRemaining()) {
				// Eat into the order
				if (Units::ScaleDown(order.GetRemaining() * price) > remainingBalance) {
					throw Error(Error::Type::InsufficientFunds,
					"User doesn't have enough money to fulfil the market buy order");
				} else {
					return true;
				}
			} else {
				// Carry onto next order
				if (Units::ScaleDown(sellOrder.GetRemaining() * price) > remainingBalance) {
					throw Error(Error::Type::InsufficientFunds,
					"User doesn't have enough money to fulfil the market buy order");
				} else {
					remainingBalance -= price * sellOrder.GetRemaining();
					order.AddToFill(sellOrder.GetRemaining());
				}
			}
		}
	}

	return (order.GetRemaining() == 0);
}

template <>
void Market::ThrowIfInsufficientFunds<OrderAction::Buy>(
const OrderContainer<MarketOrder>& orderContainer, int64_t availableBalance) const {
	SimulateMarketBuyOrder(orderContainer.order, availableBalance);
}

template <>
void Market::ThrowIfInsufficientFunds<OrderAction::Buy>(
const OrderContainer<LimitOrder>& orderContainer, int64_t availableBalance) const {
	auto funds = orderContainer.order.GetRemaining() * orderContainer.GetPrice();
	if (Units::ScaleDown(funds) > availableBalance) {
		throw Error(Error::Type::InsufficientFunds,
		"User doesn't have enough coins to make this limit order");
	}
}

template <>
void Market::ThrowIfInsufficientFunds<OrderAction::Buy>(
const OrderContainer<StopLimitOrder>& orderContainer, int64_t availableBalance) const {
	auto funds = orderContainer.order.GetRemaining() * orderContainer.order.GetActualPrice();
	if (Units::ScaleDown(funds) > availableBalance) {
		throw Error(Error::Type::InsufficientFunds,
		"User doesn't have enough coins to make this stop-limit order");
	}
}

template <>
void Market::CancelOrder<OrderAction::Buy, LimitOrder>(int64_t id, int64_t price,
MarketWallets* marketWallets) {
	CancelHelper<OrderAction::Buy, LimitOrder>(buyLimitOrderMap, id, price, marketWallets);
}

template <>
void Market::CancelOrder<OrderAction::Sell, LimitOrder>(int64_t id, int64_t price,
MarketWallets* marketWallets) {
	CancelHelper<OrderAction::Sell, LimitOrder>(sellLimitOrderMap, id, price, marketWallets);
}

template <>
void Market::CancelOrder<OrderAction::Buy, StopLimitOrder>(int64_t id, int64_t price,
MarketWallets* marketWallets) {
	CancelHelper<OrderAction::Buy, StopLimitOrder>(buyStopLimitOrderMap, id, price, marketWallets);
}

template <>
void Market::CancelOrder<OrderAction::Sell, StopLimitOrder>(int64_t id, int64_t price,
MarketWallets* marketWallets) {
	CancelHelper<OrderAction::Sell, StopLimitOrder>(sellStopLimitOrderMap, id, price, marketWallets);
}

// This should only be called for a single remove.
template <OrderAction Side, class Order, class Comp>
void Market::CancelHelper(OrderMap<Order, Comp>& orderMap, int64_t id, int64_t price,
MarketWallets* marketWallets) {
	auto ordersIter = orderMap.find(price);
	if (ordersIter == orderMap.end()) {
		throw Error(Error::Type::InvalidIdPrice, "Could not find id and rate combo");
	}

	auto& orders = ordersIter->second;

	auto limitOrderIter = std::find_if(orders.begin(), orders.end(), [id](const auto& limitOrder) {
		return (id == limitOrder.GetId());
	});

	bool foundOrder = (limitOrderIter != orders.end());
	if (!foundOrder) {
		throw Error(Error::Type::InvalidIdPrice, "Could not find id and rate combo");
	}

	auto userId = limitOrderIter->GetUserId();

	// Update wallet
	if constexpr (Side == OrderAction::Buy) {
		auto address = marketWallets->baseWallet->GetAddress(userId);
		address->RemoveFromInOrder(limitOrderIter->GetRemaining());
	} else {
		auto address = marketWallets->coinWallet->GetAddress(userId);
		address->RemoveFromInOrder(limitOrderIter->GetRemaining());
	}

	// If this price point will become empty after removing this order,
	// just remove the whole price, otherwise remove the item.
	if (orders.size() == 1) {
		orderMap.erase(ordersIter);
	} else {
		orders.erase(limitOrderIter);
	}

	// Remove from user id cache..
	auto& allUserOrders = userOrderMap.at(userId);
	auto& userOrders = GetUserOrderCacheHelper<Side, Order>(&allUserOrders);

	Comp comp;
	auto itPair = std::equal_range(userOrders.begin(), userOrders.end(),
	PriceOrderId{ price, id }, CompareUserOrders(comp));

	if (itPair.first != itPair.second) { // TODO: Is this needed if there is nothing to remove?
		userOrders.erase(itPair.first, itPair.second);
	}
}

template <OrderAction Side, class Order, typename Comp>
void Market::CancelOrders(OrderMap<Order, Comp>& orderMap, std::vector<PriceOrderId>& priceOrderIds) {
	for (auto& priceOrderId : priceOrderIds) {
		auto ordersIter = orderMap.find(priceOrderId.price);
		if (ordersIter == orderMap.end()) {
			throw Error(Error::Type::InvalidIdPrice, "Could not find id and rate combo");
		}

		auto& orders = ordersIter->second;

		auto limitOrderIter = std::find_if(orders.begin(), orders.end(), [id = priceOrderId.orderId](const auto& limitOrder) {
			return (id == limitOrder.GetId());
		});

		bool foundOrder = (limitOrderIter != orders.end());
		if (!foundOrder) {
			throw Error(Error::Type::InvalidIdPrice, "Could not find id and rate combo");
		}

		// If this price point will become empty after removing this order,
		// just remove the whole price, otherwise remove the item.
		if (orders.size() == 1) {
			orderMap.erase(ordersIter);
		} else {
			orders.erase(limitOrderIter);
		}
	}
}

void Market::CancelAll() {
	buyLimitOrderMap.clear();
	sellLimitOrderMap.clear();

	buyStopLimitOrderMap.clear();
	sellStopLimitOrderMap.clear();

	userOrderMap.clear();
}

// Returns a collection of the ids cancelled
std::vector<int64_t> Market::CancelAll(int32_t userId) {
	auto& userOrderPriceIds = GetUserOrders(userId);

	CancelOrders<OrderAction::Buy, LimitOrder>(buyLimitOrderMap, userOrderPriceIds.buyLimitPrices);
	CancelOrders<OrderAction::Sell, LimitOrder>(sellLimitOrderMap, userOrderPriceIds.sellLimitPrices);
	CancelOrders<OrderAction::Buy, StopLimitOrder>(buyStopLimitOrderMap, userOrderPriceIds.buyStopLimitPrices);
	CancelOrders<OrderAction::Sell, StopLimitOrder>(sellStopLimitOrderMap, userOrderPriceIds.sellStopLimitPrices);

	std::vector<int64_t> ids;
	auto CancelUserOrders = [&ids](auto& priceIds) {
		std::transform(priceIds.begin(), priceIds.end(),
		std::back_inserter(ids), [](const auto& priceId) { return priceId.orderId; });
		priceIds.clear();
	};

	CancelUserOrders(userOrderPriceIds.buyLimitPrices);
	CancelUserOrders(userOrderPriceIds.buyStopLimitPrices);
	CancelUserOrders(userOrderPriceIds.sellLimitPrices);
	CancelUserOrders(userOrderPriceIds.sellStopLimitPrices);

	return ids;
}

template <OrderAction Side, class T>
void Market::CommitChanges(const OrderContainer<T>& orderContainer,
MarketWallets* marketWallets) {
	if constexpr (Side == OrderAction::Buy) {
		// Set in order for original order, in actual wallet.
		auto origUserId = orderContainer.order.GetUserId();
		auto origAddress = marketWallets->baseWallet->GetAddress(origUserId);

		CommitChangesHelper<Side>(orderContainer.order.GetRemaining(), buyLimitOrderMap, sellLimitOrderMap,
		buyStopLimitOrderMap, origAddress, marketWallets);
	} else {
		auto origUserId = orderContainer.order.GetUserId();
		auto origAddress = marketWallets->coinWallet->GetAddress(origUserId);

		CommitChangesHelper<Side>(orderContainer.order.GetRemaining(), sellLimitOrderMap, buyLimitOrderMap,
		sellStopLimitOrderMap, origAddress, marketWallets);
	}
}

template <OrderAction Side, class Comp, class Order>
void Market::RemoveOrders(int32_t userId, int64_t price, int64_t stopOrderId) {
	auto it = userOrderMap.find(userId);
	if (it == userOrderMap.end()) {
		return;
	}

	auto& userOrders = it->second;
	auto& orders = GetUserOrderCacheHelper<Side, Order>(&userOrders);

	Comp comp;
	// Find all with the same price, as the user may have a few stop-limit orders at the same price.
	auto itPair = std::equal_range(orders.begin(), orders.end(),
	PriceOrderId{ price, stopOrderId }, CompareUserOrders(comp));

	if (itPair.first != itPair.second) { // TODO: Is this needed if there is nothing to remove?
		orders.erase(itPair.first, itPair.second);
	}
}

template <OrderAction Side, class Order, class Comp>
void Market::AddToUserCache(int32_t userId, int64_t price, int64_t orderId) {
	PriceOrderId priceOrderId{ price, orderId };
	auto it = userOrderMap.find(userId);
	if (it == userOrderMap.end()) {
		// TODO: Add it...
		auto itPair = userOrderMap.insert({ userId, UserOrders() });
		it = itPair.first; // Assume it was inserted....
	}

	auto& userOrders = it->second;
	auto& orders = GetUserOrderCacheHelper<Side, Order>(&userOrders);

	Comp comp;
	auto lb = std::lower_bound(orders.begin(), orders.end(), priceOrderId, CompareUserOrders(comp));
	orders.insert(lb, priceOrderId);
}

// This original order which sparked this off..
template <OrderAction Side, class Comp, class Comp1>
void Market::CommitChangesHelper(int64_t amountRemaining,
LimitOrderMap<Comp>& insertedLimitOrderMap,
LimitOrderMap<Comp1>& updatedLimitOrderMap,
StopLimitOrderMap<Comp1>& stopOrderMap,
std::vector<Address>::iterator origAddress,
MarketWallets* marketWallets) {
	// Remove from stop orders.. (TODO, double check.., test with only 1 stop order..)
	auto numTriggeredStopOrders = simulator.GetNumTriggeredStopOrders();
	for (auto it = stopOrderMap.begin(); it != stopOrderMap.end();) {
		auto& stopOrders = it->second;
		auto price = it->first;
		auto count = static_cast<int>(stopOrders.size());
		if (numTriggeredStopOrders >= count) {
			// Remove any from user's own cached orders..
			for (const auto& stopOrder : stopOrders) {
				RemoveOrders<Side, Comp1, StopLimitOrder>(stopOrder.GetUserId(), price,
				stopOrder.GetId());
			}

			// Remove the whole price point.
			it = stopOrderMap.erase(it);
			numTriggeredStopOrders -= count;
		} else {
			// Remove any from user's own cache
			auto it = stopOrders.begin();
			for (; it != stopOrders.begin() + numTriggeredStopOrders; ++it) {
				RemoveOrders<Side, Comp1, StopLimitOrder>(it->GetUserId(), price, it->GetId());
			}

			// Remove stop orders from price point
			stopOrders.erase(stopOrders.begin(), stopOrders.begin() + numTriggeredStopOrders);
			break;
		}
	}

	// Insert orders to the limit orders
	auto& simulatorInsertedLimitOrderMap = simulator.GetInsertedLimitOrders();
	for (auto& [price, insertedLimitOrders] : simulatorInsertedLimitOrderMap) {
		for (const auto& limitOrder : insertedLimitOrders) {
			insertedLimitOrderMap[price].push_back(limitOrder);
			AddToUserCache<Side, LimitOrder, Comp>(limitOrder.GetUserId(), price, limitOrder.GetId());
		}
	}

	// Insert stop limit order (is mutally exclusive with the other orders).
	if (simulator.InsertedAStopLimitOrder()) {
		auto& insertedStopLimitOrder = *simulator.GetInsertedStopLimitOrder().stopLimitOrder;
		auto price = simulator.GetInsertedStopLimitOrder().price;
		stopOrderMap[price].push_back(insertedStopLimitOrder);

		auto userId = insertedStopLimitOrder.GetUserId();
		auto orderId = insertedStopLimitOrder.GetId();
		AddToUserCache<Side, StopLimitOrder, Comp1>(userId, price, orderId);
	}

	// Remove limit orders which have been consumed
	auto numLimitOrdersToRemove = simulator.GetNumLimitOrdersToRemove();
	if (numLimitOrdersToRemove > 0) {
		for (auto it = updatedLimitOrderMap.begin(); it != updatedLimitOrderMap.end();) {
			auto& updatedLimitOrders = it->second;
			auto price = it->first;
			auto count = static_cast<int>(updatedLimitOrders.size());
			if (numLimitOrdersToRemove >= count) {
				// Remove any from user's own cached orders..
				for (const auto& limitOrder : updatedLimitOrders) {
					RemoveOrders<Side, Comp1, LimitOrder>(limitOrder.GetUserId(), price,
					limitOrder.GetId());
				}

				// Remove the whole price point.
				it = updatedLimitOrderMap.erase(it);
				numLimitOrdersToRemove -= count;
			} else {
				// Remove any from user's own cache
				for (auto it = updatedLimitOrders.begin();
				     it != updatedLimitOrders.begin() + numTriggeredStopOrders; ++it) {
					RemoveOrders<Side, Comp1, LimitOrder>(it->GetUserId(), price, it->GetId());
				}

				// Remove stop orders from price point
				updatedLimitOrders.erase(updatedLimitOrders.begin(),
				updatedLimitOrders.begin() + numLimitOrdersToRemove);
				break;
			}
		}
	}

	// Update the fill of the last limit order if needed
	if (simulator.GetLastFill() > 0) {
		updatedLimitOrderMap.begin()->second.front().AddToFill(simulator.GetLastFill());
	}

	origAddress->AddToInOrder(amountRemaining);

	// Update balances for all the address for which trades occurred
	const auto& allTrades = simulator.GetTrades();
	for (const auto& trade : allTrades) {
		auto buyersCoinAddress = marketWallets->coinWallet->GetAddress(trade.buyUserId);
		buyersCoinAddress->AddToTotalBalance(trade.amount - (trade.fees.buyFee));

		// Remove the coin they used to buy from their balance
		auto buyersBaseAddress = marketWallets->baseWallet->GetAddress(trade.buyUserId);
		buyersBaseAddress->RemoveFromTotalBalance(Units::ScaleDown(trade.amount * trade.price));
		buyersBaseAddress->RemoveFromInOrder(Units::ScaleDown(trade.amount * trade.price));

		// Now do same but for seller
		auto sellersBaseAddress = marketWallets->baseWallet->GetAddress(trade.sellUserId);
		sellersBaseAddress->AddToTotalBalance(trade.amount - (trade.fees.sellFee));

		// Remove the amount of coin they used to sell from the balance
		auto sellersCoinAddress = marketWallets->coinWallet->GetAddress(trade.sellUserId);
		sellersCoinAddress->RemoveFromTotalBalance(trade.amount);
		sellersCoinAddress->RemoveFromInOrder(trade.amount);
	}
}

const BuyLimitOrderMap& Market::GetBuyLimitOrderMap() const {
	return buyLimitOrderMap;
}

const SellLimitOrderMap& Market::GetSellLimitOrderMap() const {
	return sellLimitOrderMap;
}

const BuyStopLimitOrderMap& Market::GetBuyStopLimitOrderMap() const {
	return buyStopLimitOrderMap;
}

const SellStopLimitOrderMap& Market::GetSellStopLimitOrderMap() const {
	return sellStopLimitOrderMap;
}

const CoinPair& Market::GetCoinPair() const {
	return coinPair;
}

bool Market::operator==(const Market& market) const {
	return coinPair == market.coinPair && buyLimitOrderMap == market.buyLimitOrderMap
	&& sellLimitOrderMap == market.sellLimitOrderMap
	&& buyStopLimitOrderMap == market.buyStopLimitOrderMap
	&& sellStopLimitOrderMap == market.sellStopLimitOrderMap
	&& currentOrderId == market.currentOrderId && currentTradeId == market.currentTradeId
	&& userOrderMap == market.userOrderMap && config == market.config
	&& *listener == *market.listener;
}

void Market::SetMaxOrderId(int64_t id) {
	currentOrderId = id;
}

void Market::SetMaxTradeId(int64_t id) {
	currentTradeId = id;
}

IListener& Market::GetListener() const {
	return *listener;
}

void Market::SetFeePercentage(double feePercent) {
	config.feeDivision = Fee::ConvertToDivisibleFee(feePercent);
}

void Market::SetMaxNumLimitOpenOrders(int32_t numOpenOrders) {
	config.maxNumLimitOpenOrders = numOpenOrders;
}

void Market::SetMaxNumStopLimitOpenOrders(int32_t numOpenOrders) {
	config.maxNumStopLimitOpenOrders = numOpenOrders;
}

Fee Market::CalculateFees(int64_t amount, int64_t price) const {
	auto standardFee = config.feeDivision;

	Fee fees;
	fees.buyFee = amount / standardFee;
	fees.sellFee = Units::ScaleDown(amount * price) / standardFee;
	return fees;
}

LimitOrder Market::ConvertToLimitOrder(const StopLimitOrder& stopLimitOrder) const {
	// The stop limit order becomes a limit order
	LimitOrder limitOrder{ stopLimitOrder.GetUserId(), stopLimitOrder.GetAmount(),
		stopLimitOrder.GetFilled() };
	limitOrder.SetId(stopLimitOrder.GetId());
	return limitOrder;
}

UserOrders& Market::GetUserOrders(int32_t userId) {
	return userOrderMap.at(userId);
}

template <OrderAction Side, class Order>
const std::vector<PriceOrderId>& Market::GetUserOrderCache(int32_t userId) {
	auto& userOrders = GetUserOrders(userId);
	return GetUserOrderCacheHelper<Side, Order>(&userOrders);
}

template <OrderAction Side, class Order>
std::vector<PriceOrderId>& Market::GetUserOrderCacheHelper(UserOrders* userOrders) {
	if constexpr (Side == OrderAction::Buy) {
		if constexpr (IsLimitOrder_v<Order>) {
			return userOrders->buyLimitPrices;
		} else {
			return userOrders->buyStopLimitPrices;
		}
	} else {
		if constexpr (IsLimitOrder_v<Order>) {
			return userOrders->sellLimitPrices;
		} else {
			return userOrders->sellStopLimitPrices;
		}
	}
}

const MarketConfig& Market::GetConfig() const {
	return config;
}

const UserOrderMap& Market::GetUserOrderMap() const {
	return userOrderMap;
}

template <OrderAction Side, class Order, class Comp>
void Market::ForceAdd(OrderMap<Order, Comp>& orderMap,
const OrderContainer<Order>& orderContainer) {
	// Add to order map
	orderMap[orderContainer.GetPrice()].push_back(orderContainer.order);
	AddToUserCache<Side, Order, Comp>(orderContainer.order.GetUserId(), orderContainer.GetPrice(),
	orderContainer.order.GetId());
}

template <OrderAction Side, class Order>
void Market::ForceAddOrder(const OrderContainer<Order>& orderContainer) {
	if constexpr (Side == OrderAction::Buy) {
		if constexpr (IsLimitOrder_v<Order>) {
			ForceAdd<Side>(buyLimitOrderMap, orderContainer);
		} else {
			ForceAdd<Side>(buyStopLimitOrderMap, orderContainer);
		}
	} else {
		if constexpr (IsLimitOrder_v<Order>) {
			ForceAdd<Side>(sellLimitOrderMap, orderContainer);
		} else {
			ForceAdd<Side>(sellStopLimitOrderMap, orderContainer);
		}
	}
}

// Explicit instantiations for public methods (so I can leave the definitions in .cpp file)
template void Market::NewProcess<OrderAction::Buy>(
const OrderContainer<MarketOrder>& orderContainer, MarketWallets* marketWallets);
template void Market::NewProcess<OrderAction::Sell>(
const OrderContainer<MarketOrder>& orderContainer, MarketWallets* marketWallets);
template void Market::NewProcess<OrderAction::Buy>(
const OrderContainer<LimitOrder>& orderContainer, MarketWallets* marketWallets);
template void Market::NewProcess<OrderAction::Sell>(
const OrderContainer<LimitOrder>& orderContainer, MarketWallets* marketWallets);
template void Market::NewProcess<OrderAction::Buy>(
const OrderContainer<StopLimitOrder>& orderContainer, MarketWallets* marketWallets);
template void Market::NewProcess<OrderAction::Sell>(
const OrderContainer<StopLimitOrder>& orderContainer, MarketWallets* marketWallets);

template const std::vector<PriceOrderId>& Market::GetUserOrderCache<OrderAction::Buy, LimitOrder>(
int32_t userId);
template const std::vector<PriceOrderId>& Market::GetUserOrderCache<OrderAction::Sell,
LimitOrder>(int32_t userId);
template const std::vector<PriceOrderId>& Market::GetUserOrderCache<OrderAction::Buy,
StopLimitOrder>(int32_t userId);
template const std::vector<PriceOrderId>& Market::GetUserOrderCache<OrderAction::Sell,
StopLimitOrder>(int32_t userId);

template void Market::ForceAddOrder<OrderAction::Buy>(
const OrderContainer<LimitOrder>& orderContainer);
template void Market::ForceAddOrder<OrderAction::Sell>(
const OrderContainer<LimitOrder>& orderContainer);
template void Market::ForceAddOrder<OrderAction::Buy>(
const OrderContainer<StopLimitOrder>& orderContainer);
template void Market::ForceAddOrder<OrderAction::Sell>(
const OrderContainer<StopLimitOrder>& orderContainer);
