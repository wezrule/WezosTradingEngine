#include <TradingEngine/Fee.h>
#include <gtest/gtest.h>

TEST(TestFees, ConvertToDivisible) {
	ASSERT_EQ(Fee::ConvertToDivisibleFee(0.1), 1000);
}

TEST(TestFees, ConvertToFeePercent) {
	ASSERT_EQ(Fee::ConvertToFeePercent(1000), 0.1);
}
