add_library (trading_engine
	Address.cpp
	Address.h
	CoinPair.h
	Error.h
	FatalError.h
	Fee.h
	IWallet.h
	Listener/IListener.h
	Listener/Listener.cpp
	Listener/Listener.h
	Listener/ListenerOrder.cpp
	Listener/ListenerOrder.h
	Listener/Operation.cpp
	Listener/Operation.h
	Market.cpp
	Market.h
	market_helper.h
	MarketManager.cpp
	MarketManager.h
	Message.h
	MessageType.h
	Orders/BaseOrder.cpp
	Orders/BaseOrder.h
	Orders/LimitOrder.cpp
	Orders/LimitOrder.h
	Orders/MarketOrder.cpp
	Orders/MarketOrder.h
	Orders/OrderAction.h
	Orders/OrderContainer.h
	Orders/OrderType.h
	Orders/StopLimitOrder.cpp
	Orders/StopLimitOrder.h
	PlatformSpecific/allocator_constants
	PoolAlloc.h
	serializer_defines.h
	SharedPoolAllocator.h
	Simulator.cpp
	Simulator.h
	SimulatorTrade.h
	TradingEngine.cpp
	TradingEngine.h
	Units.h
	Wallet.cpp
	Wallet.h
	WalletManager.cpp
	WalletManager.h)

INCLUDE_DIRECTORIES (${Boost_INCLUDE_DIR})