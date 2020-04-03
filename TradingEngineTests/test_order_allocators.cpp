#include <TradingEngine/Orders/StopLimitOrder.h>
#include <TradingEngine/PlatformSpecific/allocator_constants.h>
#include <TradingEngine/market_helper.h>
#include <algorithm>
#include <deque>
#include <gtest/gtest.h>
#include <stddef.h>
#include <type_traits>
#include <vector>

template <class T>
class NewDeleteAllocator {
public:
	typedef T value_type;

	//NewDeleteAllocator() = default;
	NewDeleteAllocator(std::vector<size_t>* allocated) :
	allocated(allocated) {
	}

	template <typename U>
	NewDeleteAllocator(const NewDeleteAllocator<U>& a) {
		allocated = a.allocated;
	}

	NewDeleteAllocator(const NewDeleteAllocator& a) {
		allocated = a.allocated;
	}
	NewDeleteAllocator& operator=(const NewDeleteAllocator& a) {
		allocated = a.allocated;
		return *this;
	}
	NewDeleteAllocator(NewDeleteAllocator&&) = default;
	NewDeleteAllocator& operator=(NewDeleteAllocator&&) = default;

	typedef std::true_type propagate_on_container_move_assignment;
	typedef std::true_type is_always_equal;

	T* allocate(size_t numToAllocate) {
		allocated->push_back(numToAllocate);
		return static_cast<value_type*>(::operator new(sizeof(T) * numToAllocate));
	}

	void deallocate(T* p, size_t numToDeallocate) {
		::operator delete(p);
	}

	std::vector<size_t>* allocated;
};

template <class T, class U>
bool operator==(const NewDeleteAllocator<T>&, const NewDeleteAllocator<U>&) noexcept {
	return true;
}

template <class T, class U>
bool operator!=(const NewDeleteAllocator<T>& x, const NewDeleteAllocator<U>& y) noexcept {
	return !(x == y);
}

size_t FindModeAllocNumber(std::vector<size_t>& allocated) {
	std::sort(allocated.begin(), allocated.end());

	size_t count = 0;
	auto curNum = allocated.front();
	size_t modeAllocNum = curNum;
	size_t highestCount = 0;

	for (auto numAllocated : allocated) {
		if (curNum == numAllocated) {
			++count;
		} else {
			if (count > highestCount) {
				modeAllocNum = curNum;
				highestCount = count;
			}

			curNum = numAllocated;
			count = 1;
		}
	}

	return modeAllocNum;
}

TEST(TestOrderAllocator, defaults) {
	std::vector<size_t> allocated;
	NewDeleteAllocator<LimitOrder> a(&allocated);
	BaseOrders<LimitOrder, NewDeleteAllocator<LimitOrder>> orders(a);

	size_t numOrdersToAdd = 1000;
	for (size_t i = 0; i < numOrdersToAdd; ++i) {
		orders.emplace_back(1, 2, 3);
	}

	ASSERT_EQ(FindModeAllocNumber(allocated), ps::GetOrderDequeSize<LimitOrder>());

	allocated.clear();
	NewDeleteAllocator<StopLimitOrder> a1(&allocated);
	BaseOrders<StopLimitOrder, NewDeleteAllocator<StopLimitOrder>> stopLimitOrders(a1);

	for (size_t i = 0; i < numOrdersToAdd; ++i) {
		stopLimitOrders.emplace_back(1, 2, 3, 1);
	}

	ASSERT_EQ(FindModeAllocNumber(allocated), ps::GetOrderDequeSize<StopLimitOrder>());
}
