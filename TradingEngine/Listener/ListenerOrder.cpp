#include "ListenerOrder.h"

// The reason for id1 & id2 is that it could be a number of id's based on the union of ListenerOrder
ListenerOrder::ListenerOrder(int64_t tradeId, int64_t id1, int64_t id2, int64_t amount,
int64_t filled, int64_t price, int64_t actualPrice, OrderType type) :
tradeId(tradeId),
orderId(id1),
userId(id2),
amount(amount),
filled(filled),
price(price),
actualPrice(actualPrice),
orderType(type) {
}

ListenerOrder::ListenerOrder(int64_t id1, int64_t id2, int64_t amount, int64_t filled,
int64_t price, int64_t actualPrice, OrderType type) :
orderId(id1),
userId(id2),
amount(amount),
filled(filled),
price(price),
actualPrice(actualPrice),
orderType(type) {
}

bool ListenerOrder::operator==(const ListenerOrder& listenerOrder) const {
	return tradeId == listenerOrder.tradeId && orderId == listenerOrder.orderId
	&& userId == listenerOrder.userId && amount == listenerOrder.amount
	&& filled == listenerOrder.filled && price == listenerOrder.price
	&& actualPrice == listenerOrder.actualPrice && fees == listenerOrder.fees;
}

template <>
ListenerOrder ConvertToListenerOrder(const OrderContainer<LimitOrder>& limitOrderContainer) {
	return ListenerOrder{ limitOrderContainer.order.GetId(),
		limitOrderContainer.order.GetUserId(),
		limitOrderContainer.order.GetAmount(),
		limitOrderContainer.order.GetFilled(), limitOrderContainer.GetPrice(),
		-1, OrderType::Limit };
}

template <>
ListenerOrder ConvertToListenerOrder(const OrderContainer<MarketOrder>& marketOrderContainer) {
	auto& marketOrder = marketOrderContainer.order;
	return ListenerOrder{ marketOrder.GetId(), marketOrder.GetUserId(), marketOrder.GetAmount(),
		marketOrder.GetFilled(), -1, -1, OrderType::Market };
}

template <>
ListenerOrder ConvertToListenerOrder(
const OrderContainer<StopLimitOrder>& stopLimitOrderContainer) {
	return ListenerOrder{ stopLimitOrderContainer.order.GetId(),
		stopLimitOrderContainer.order.GetUserId(),
		stopLimitOrderContainer.order.GetAmount(),
		stopLimitOrderContainer.order.GetFilled(),
		stopLimitOrderContainer.GetPrice(),
		stopLimitOrderContainer.order.GetActualPrice(), OrderType::StopLimit };
}
