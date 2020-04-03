#pragma once

// DO NOT CHANGE THE ORDER!! well except Last should be at the end...
enum class MessageType {
	MarketOrder = 0,
	LimitOrder,
	StopLimitOrder,

	CancelOrder,
	CancelAllOrders,
	Deposit,
	Withdraw,

	NewCoin,
	NewMarket,

	SetFeePercentage,
	SetMaxNumLimitOpenOrders,
	SetMaxNumStopLimitOpenOrders,

	SetInOrder,
	GetAmount,
	GetAvailable,
	GetInOrder,
	GetTotal,

	ClearOpenOrders,
	ClearEveryonesOpenOrders,
	ClearAllEveryonesOpenOrders,

	// Output only messages
	NewOpenOrder,
	NewTrade,
	OrderFilled,
	NewFilledOrder,
	PartialFill,
	StopLimitTriggered,

	Quit,

	// This should be at the end...
	Last = 999999
};
