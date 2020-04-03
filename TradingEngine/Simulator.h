#pragma once

#include "Orders/StopLimitOrder.h"
#include "SimulatorTrade.h"

#include <cstdint>
#include <map>
#include <memory>
#include <vector>

struct PriceStopLimitOrder {
	int64_t price;
	std::unique_ptr<StopLimitOrder> stopLimitOrder; // Should at max be one...

	PriceStopLimitOrder& operator=(const PriceStopLimitOrder& priceStopLimitOrder) {
		price = priceStopLimitOrder.price;
		if (stopLimitOrder) {
			stopLimitOrder = std::make_unique<StopLimitOrder>(*stopLimitOrder);
		}
		return *this;
	}
};

class Simulator {
public:
	void AddTrade(int32_t buyUserId, int32_t sellUserId, int64_t amount, int64_t price, const Fee& fees);
	const std::vector<SimulatorTrade>& GetTrades() const;
	void Clear();

	void SetCurrentOrderId(int64_t orderId);
	void IncrementOrderId();
	int64_t GetCurrentOrderId() const;

	void SetCurrentTradeId(int64_t tradeId);
	void IncrementTradeId();
	int64_t GetCurrentTradeId() const;

	int GetNumTriggeredStopOrders() const;
	void IncrementNumTriggeredStopOrders();

	int GetNumLimitOrdersToRemove() const;
	void IncrementNumLimitOrdersToRemove();

	void SetLastFill(int64_t lastFill);
	int64_t GetLastFill();

	const std::map<int64_t, std::vector<LimitOrder>>& GetInsertedLimitOrders() const;
	void InsertLimitOrder(int64_t price, const LimitOrder& limitOrder);

	bool InsertedAStopLimitOrder() const;
	void SetInsertedStopLimitOrder(int64_t price, const StopLimitOrder& stopLimitOrder);
	const PriceStopLimitOrder& GetInsertedStopLimitOrder() const;
	void AddToLastFill(int64_t fill);

	inline Simulator& operator=(const Simulator& simulator) = default;

private:
	int64_t currentOrderId = -1; // Before making any changes
	int64_t currentTradeId = -1; // Before making any changes

	std::map<int64_t, std::vector<LimitOrder>> insertedLimitOrderMap;
	PriceStopLimitOrder insertedStopLimitOrder; // Should at max be one...
	std::vector<SimulatorTrade> trades;

	int numTriggeredStopOrders = 0;

	int64_t lastFill = 0;
	int numLimitOrdersToRemove = 0;
};
