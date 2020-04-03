#pragma once

#include <cstdint>

// Convert without loss of precision from a double to a 64-bit integer.
// Taken from https://en.bitcoin.it/wiki/Proper_Money_Handling_(JSON-RPC) for bitcoin

// These are made templates so that we can prevent
// implicit primitive conversions in tests accidentally
class Units {
public:
	// Only used with tests
	template <class T>
	static int64_t ExToIn(T value) {
		static_assert(std::is_same_v<double, T>);
		return static_cast<int64_t>(value * scale + 0.5);
	}

	// This takes the result of a multiplication between 2 scales integers,
	// and reduces them by the desired scale
	template <class T>
	static int64_t ScaleDown(T doubleScaledInt) {
		static_assert(std::is_same_v<T, int64_t>);
		return doubleScaledInt / scale;
	}

private:
	constexpr static double scale = 1e8;
};
