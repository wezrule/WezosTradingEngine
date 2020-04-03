add_executable (trading_engine_tests
	gtest_main.cpp
	message_conversion_testing_helper.h
	MessageConversion.h
	mock_listener.h
	sample_ecs_test.h
	StubListener.h
	StubMarketConfig.cpp
	StubWallet.h
	test_address.cpp
	test_cancel_order.cpp
	test_coin_pair.cpp
	test_empty_market_making_market_orders.cpp
	test_error.cpp
	test_fees.cpp
	test_invalid_stop_rate.cpp
	test_limit_only_market_trade.cpp
	test_market.cpp
	test_market_listener.cpp
	test_market_manager.cpp
	test_market_sorting.cpp
	test_maximum_orders.cpp
	test_messagetype_enum.cpp
	test_mixed_limit_only.cpp
	test_only_limit_stop_limit.cpp
	test_only_stop_order.cpp
	test_order_allocators.cpp
	test_order_construction.cpp
	test_simulator.cpp
	test_trade_same_user.cpp
	test_trading_engine.cpp
	test_user_order_cache.cpp
	test_wallet.cpp
	test_wallet_manager.cpp
)

target_link_libraries (trading_engine_tests
	trading_engine
	gtest
	gmock
	Boost::boost
	Boost::serialization)

add_test(NAME trading_engine_tests_all COMMAND trading_engine_tests)
