#include "Simulator.h"

void Simulator::AddTrade(int32_t buyUserId, int32_t sellUserId, int64_t amount, int64_t price,
const Fee& fees) {
	trades.emplace_back(buyUserId, sellUserId, amount, price, fees);
}

const std::vector<SimulatorTrade>& Simulator::GetTrades() const {
	return trades;
}

void Simulator::SetCurrentOrderId(int64_t orderId) {
	currentOrderId = orderId;
}

void Simulator::IncrementOrderId() {
	++currentOrderId;
}

int64_t Simulator::GetCurrentOrderId() const {
	return currentOrderId;
}

void Simulator::SetCurrentTradeId(int64_t tradeId) {
	currentTradeId = tradeId;
}

void Simulator::IncrementTradeId() {
	++currentTradeId;
}

int64_t Simulator::GetCurrentTradeId() const {
	return currentTradeId;
}

void Simulator::IncrementNumTriggeredStopOrders() {
	++numTriggeredStopOrders;
}

int Simulator::GetNumTriggeredStopOrders() const {
	return numTriggeredStopOrders;
}

void Simulator::IncrementNumLimitOrdersToRemove() {
	++numLimitOrdersToRemove;
}

int Simulator::GetNumLimitOrdersToRemove() const {
	return numLimitOrdersToRemove;
}

void Simulator::SetLastFill(int64_t lastFill) {
	this->lastFill = lastFill;
}

int64_t Simulator::GetLastFill() {
	return lastFill;
}

const std::map<int64_t, std::vector<LimitOrder>>& Simulator::GetInsertedLimitOrders() const {
	return insertedLimitOrderMap;
}

void Simulator::InsertLimitOrder(int64_t price, const LimitOrder& limitOrder) {
	insertedLimitOrderMap[price].push_back(limitOrder);
}

bool Simulator::InsertedAStopLimitOrder() const {
	return (insertedStopLimitOrder.stopLimitOrder != nullptr);
}

void Simulator::SetInsertedStopLimitOrder(int64_t price, const StopLimitOrder& stopLimitOrder) {
	insertedStopLimitOrder.price = price;
	insertedStopLimitOrder.stopLimitOrder = std::make_unique<StopLimitOrder>(stopLimitOrder);
}

const PriceStopLimitOrder& Simulator::GetInsertedStopLimitOrder() const {
	return insertedStopLimitOrder;
}

void Simulator::AddToLastFill(int64_t fill) {
	lastFill += fill;
}

void Simulator::Clear() {
	trades.clear();

	currentOrderId = -1;
	currentTradeId = -1;
	insertedLimitOrderMap.clear();
	insertedStopLimitOrder.stopLimitOrder = nullptr;
	insertedStopLimitOrder.price = -1;

	numTriggeredStopOrders = 0;

	lastFill = 0;
	numLimitOrdersToRemove = 0;
}
