#ifndef SRC_BOUNDEDBUFFER_H_
#define SRC_BOUNDEDBUFFER_H_

#include <algorithm>
#include <memory>

#include <utility>
#include <stdexcept>

// \TODO Removem
#include <iostream>

template <typename T>
struct BoundedBuffer {
	using container_type = T*;
	using value_type = T;
	using reference = value_type &;
	using const_reference = value_type const &;
	using size_type = size_t;

	BoundedBuffer(size_type n) : n_{n} {
		if (n <= 0) throw std::invalid_argument{"n must be > 0"};
		container_ = new T[n_];
	}
	BoundedBuffer(BoundedBuffer const & other) :
		index_{other.index_}, cnt_{other.cnt_}, n_{other.n_}, container_{new T[other.n_]} {
			copyFromBufferToArray(other, container_);
	}

	BoundedBuffer(BoundedBuffer && other) :
		index_{std::move(other.index_)}, cnt_{std::move(other.cnt_)}, n_{std::move(other.n_)}, container_{std::move(other.container_)} {
			other.container_ = nullptr;
	}

	BoundedBuffer & operator=(BoundedBuffer const & other) {
		if (&other == this) return *this;

		index_ = other.index_;
		cnt_ = other.cnt_;
		n_ = other.n_;

		container_type tmp = new T[n_];
		copyFromBufferToArray(other, tmp);
		std::swap(container_, tmp);
		delete[] tmp;

		return *this;
	}

	BoundedBuffer & operator=(BoundedBuffer && other) {
		if (&other == this) return *this;

		index_ = other.index_;
		cnt_ = other.cnt_;
		n_ = other.n_;

		std::swap(container_, other.container_);

		return *this;
	}

	~BoundedBuffer() { delete[] container_; }

	bool empty() const noexcept { return cnt_ == 0; }
	bool full() const noexcept { return cnt_ == n_; }

	size_type size() const noexcept { return cnt_; }

	reference front() {
		throwIfEmpty();
		return _at(index_);
	}
	const_reference front() const {
		throwIfEmpty();
		return _at(index_);
	}

	reference back() {
		throwIfEmpty();
		return _at(lastIndex());
	}
	const_reference back() const {
		throwIfEmpty();
		return _at(lastIndex());
	}

	void push(T const & ele) {
		throwIfFull();
		_at(addToIndex()) = ele;
	}
	void push(T && ele) {
		throwIfFull();
		_at(addToIndex()) = std::move(ele);
	}

	void pop() {
		throwIfEmpty();
		--cnt_;
		++index_;
	}
	void swap(BoundedBuffer & b) {
		if (!(empty() && b.empty())) {
			using std::swap;
			swap(index_, b.index_);
			swap(cnt_, b.cnt_);
			swap(n_, b.n_);
			swap(container_, b.container_);
		}
	}

	template<typename FIRST, typename... REST>
	void push_many(FIRST && first, REST && ...rest) {
		push(std::forward<FIRST>(first));
		push_many(std::forward<decltype(rest)>(rest)...);
	}

	template<typename FIRST>
	void push_many(FIRST && first) {
		push(std::forward<FIRST>(first));
	}
private:
	size_type index_{0};
	size_type cnt_{0};
	size_type n_{0};
	container_type container_;

	reference _at(size_type const i) { return container_[calcMod(i)]; }
	const_reference _at(size_type const i) const { return container_[calcMod(i)]; }

	void throwIfEmpty() const { if (empty()) throw std::logic_error{"empty container"}; }
	void throwIfFull() const { if (full()) throw std::logic_error{"full container"}; }

	void copyFromBufferToArray(BoundedBuffer const & from, container_type to) {
		for (size_t i{0}; i < from.cnt_; ++i) {
			auto const pos = calcMod(from.index_ + i);
			to[pos] = from.container_[pos];
		}
	}
	size_type calcMod(size_type const & i) const noexcept { return i % n_; }
	size_type lastIndex() const noexcept { return index_ + cnt_ - 1; }
	size_type addToIndex() noexcept { return index_ + cnt_++; }
};

#endif /* SRC_BOUNDEDBUFFER_H_ */
