#pragma once

#include "gmock/gmock.h"
#include <TradingEngine/Listener/IListener.h>

class MockListener : public IListener {
public:
	MOCK_METHOD1(CancelOrder, void(int64_t id));
	MOCK_METHOD1(OrderFilled, void(int64_t id));
	MOCK_METHOD2(NewOpenOrder, void(const ListenerOrder& order, OrderAction action));
	MOCK_METHOD6(NewTrade, void(int64_t tradeId, int64_t buyOrderId, int64_t sellOrderId, int64_t amount, int64_t price, const Fee& fees));
	MOCK_METHOD2(NewFilledOrder, void(const ListenerOrder& order, OrderAction action));
	MOCK_METHOD2(PartialFill, void(int64_t id, int64_t amount));
	MOCK_METHOD2(StopLimitTriggered, void(int64_t stopLimitId, int64_t triggeredTradeId));
	MOCK_CONST_METHOD0(GetOperations, const std::vector<Operation>&());
	MOCK_METHOD0(ClearOperations, void());
	MOCK_CONST_METHOD1(Equals, bool(const IListener& listener));
	MOCK_CONST_METHOD0(Clone, std::unique_ptr<IListener>());
};
