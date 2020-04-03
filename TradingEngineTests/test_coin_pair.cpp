#include <TradingEngine/CoinPair.h>
#include <gtest/gtest.h>

TEST(TestCoinPair, equality) {
	CoinPair pair1{ 2, 1 };
	CoinPair pair2{ 2, 1 };
	CoinPair pair3{ 2, 3 };

	ASSERT_EQ(pair1, pair2);
	ASSERT_NE(pair1, pair3);
}

TEST(TestCoinPair, getters) {
	CoinPair pair{ 2, 3 };
	ASSERT_EQ(pair.GetCoinId(), 2);
	ASSERT_EQ(pair.GetBaseId(), 3);

	CoinPair pair1{ 4, 5 };
	ASSERT_EQ(pair1.GetCoinId(), 4);
	ASSERT_EQ(pair1.GetBaseId(), 5);
}
