#pragma once

// This allocator can be shared between different instances of the same containers.
// Currently it should only be used for GCC std::deque containers. There is
// no standard block size for std::deque, see TradingEngine/PlatformSpecific/allocator_constants.h
// for more information.

#include <iostream>

// https://rawgit.com/google/cxx-std-draft/allocator-paper/allocator_user_guide.html
template <class T, size_t dequeNodeSize, size_t chunkSize>
class SingletonSharedPool {
public:
	using value_type = T;

	static T* allocate(size_t numToAllocate) {
		if (numToAllocate != dequeNodeSize) {
			return static_cast<value_type*>(::operator new(sizeof(T) * numToAllocate));
		} else if (available.empty()) {
			auto numAddresses = chunkSize / dequeNodeSize;

			// Allocate another chunk
			auto toAllocate = chunkSize;
			available.reserve(available.capacity() + numAddresses);

			std::vector<T> allocated;
			allocated.reserve(toAllocate);

			memory.emplace_back(std::move(allocated));
			auto& newBlock = memory.back();
			size_t to_return = toAllocate - dequeNodeSize;
			for (size_t i = 0; i < to_return / dequeNodeSize; ++i) {
				available.push_back(std::addressof(newBlock[i * dequeNodeSize]));
			}

			// Return the address of the last one available
			return std::addressof(newBlock[to_return]);
		} else {
			T* result = available.back();
			available.pop_back();
			return result;
		}
	}

	static void deallocate(T* ptr, size_t numToFree) {
		if (numToFree == dequeNodeSize) {
			available.push_back(ptr);
		} else {
			::operator delete(ptr);
		}
	}

private:
	inline static std::vector<std::vector<T>> memory;
	inline static std::vector<T*> available;
};

template <typename T, size_t dequeNodeSize, size_t chunkSize>
class SharedPoolAllocator {
public:
	typedef T value_type;

	// This is needed as the default one provided by
	template <class U>
	struct rebind {
		using other = SharedPoolAllocator<U, dequeNodeSize, chunkSize>;
	};

	SharedPoolAllocator() noexcept = default;
	template <typename U, size_t I, size_t S>
	SharedPoolAllocator(const SharedPoolAllocator<U, I, S>&) noexcept {}

	SharedPoolAllocator(const SharedPoolAllocator&) noexcept = default;
	SharedPoolAllocator& operator=(const SharedPoolAllocator&) noexcept {
		return *this;
	}
	SharedPoolAllocator(SharedPoolAllocator&&) noexcept = default;
	SharedPoolAllocator& operator=(SharedPoolAllocator&&) noexcept = default;

	using propagate_on_container_move_assignment = std::true_type;
	using is_always_equal = std::true_type;

	value_type* allocate(size_t numToAllocate) const {
		return SingletonSharedPool<T, dequeNodeSize, chunkSize>::allocate(numToAllocate);
	}

	void deallocate(value_type* ptr, size_t numToFree) const {
		SingletonSharedPool<T, dequeNodeSize, chunkSize>::deallocate(ptr, numToFree);
	}
};

template <class T, size_t nodeSizeT, size_t chunkSizeT, class U, size_t nodeSizeU, size_t chunkSizeU>
bool operator==(const SharedPoolAllocator<T, nodeSizeT, chunkSizeT>&,
const SharedPoolAllocator<U, nodeSizeU, chunkSizeU>&) noexcept {
	return true;
}

template <class T, size_t nodeSizeT, size_t chunkSizeT, class U, size_t nodeSizeU, size_t chunkSizeU>
bool operator!=(const SharedPoolAllocator<T, nodeSizeT, chunkSizeT>& x,
const SharedPoolAllocator<U, nodeSizeU, chunkSizeU>& y) noexcept {
	return !(x == y);
}
