#pragma once

#include <cstdint>

template <class Order>
class OrderContainer {
public:
	OrderContainer<Order>(const Order& order, int64_t price) :
	order(order),
	price(price) {
	}

	Order order;
	int64_t price;

	bool operator==(const OrderContainer<Order>& container) const {
		return order == container.order && price == container.price;
	}

	int64_t GetPrice() const {
		return price;
	}
};
