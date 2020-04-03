#pragma once

#include <vector>

// This stateful allocator is useful for std::map and std::list (or any node based container).
// It is a block allocator, which doubles each time, starting at "initialSize".
// Does not support copying/moving in allocator aware containers though.
// Read this when I want to do it:
//        https://rawgit.com/google/cxx-std-draft/allocator-paper/allocator_user_guide.html
template <typename T, size_t initialSize>
struct PoolAlloc {
	using value_type = T;

	PoolAlloc() {
		// Set an initial size
		available.reserve(initialSize);

		std::vector<T> allocated;
		allocated.reserve(initialSize);
		memory.emplace_back(std::move(allocated));
		auto& newBlock = memory.back();
		for (size_t i = 0; i < initialSize; ++i) {
			available.push_back(std::addressof(newBlock[i]));
		}
	}

	// This is needed as the default one provided by allocator::traits only has 1 template argument
	template <class U>
	struct rebind {
		using other = PoolAlloc<U, initialSize>;
	};

	template <typename U, size_t I>
	PoolAlloc(const PoolAlloc<U, I>&) = delete;

	PoolAlloc(const PoolAlloc&) = delete;
	PoolAlloc& operator=(const PoolAlloc&) = delete;
	PoolAlloc(PoolAlloc&&) = delete;
	PoolAlloc& operator=(PoolAlloc&&) = delete;

	using propagate_on_container_move_assignment = std::true_type;

	T* allocate(size_t numToAllocate) {
		if (numToAllocate != 1) {
			return static_cast<T*>(::operator new(sizeof(T) * numToAllocate));
		} else if (available.empty()) {
			// Create a new block of same size, which will double the size overall.
			auto toAllocate = available.capacity();
			available.reserve(available.capacity() + toAllocate);

			std::vector<T> allocated;
			allocated.reserve(toAllocate);

			memory.emplace_back(std::move(allocated));
			size_t toReturn = toAllocate - 1;
			auto& newBlock = memory.back();
			for (size_t i = 0; i < toReturn; ++i) {
				available.push_back(std::addressof(newBlock[i]));
			}

			// Return the address of the last one available
			return std::addressof(newBlock[toReturn]);
		} else {
			T* result = available.back();
			available.pop_back();
			return result;
		}
	}

	void deallocate(value_type* ptr, size_t numToFree) {
		if (numToFree == 1) {
			available.push_back(ptr);
		} else {
			::operator delete(ptr);
		}
	}

private:
	std::vector<std::vector<T>> memory;
	std::vector<T*> available;
};

// "Equality of allocators does not imply that they must have exactly the same internal state,
// only that they must both be able to deallocate memory that was allocated with either allocator. "
// https://stackoverflow.com/questions/24278803/how-can-i-write-a-stateful-allocator-in-c11-given-requirements-on-copy-constr
template <class T, size_t initialSizeT, class U, size_t initialSizeU>
bool operator==(PoolAlloc<T, initialSizeT> const&, PoolAlloc<U, initialSizeU> const&) noexcept {
	return true;
}

template <class T, size_t initialSizeT, class U, size_t initialSizeU>
bool operator!=(PoolAlloc<T, initialSizeT> const& x, PoolAlloc<U, initialSizeU> const& y) noexcept {
	return !(x == y);
}
