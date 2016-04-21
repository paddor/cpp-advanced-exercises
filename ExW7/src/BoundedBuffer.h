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
	using index_type = size_t;

	BoundedBuffer(size_type n) : n_{n} {
		if (n <= 0) throw std::invalid_argument{"n must be > 0"};
		container_ = new T[n_];
	}
	BoundedBuffer(BoundedBuffer const & other) :
		index_{0}, cnt_{0}, n_{other.n_}, container_{new T[other.n_]} {
			for (size_t i{0}; i < other.cnt_; ++i) push(other._at(i));
	}

	BoundedBuffer(BoundedBuffer && other) :
		index_{std::move(other.index_)}, cnt_{std::move(other.cnt_)}, n_{std::move(other.n_)}, container_{std::move(other.container_)} {
			other.container_ = nullptr;
		}

	BoundedBuffer & operator=(BoundedBuffer const & other) {
		if (&other == this) return *this;
		index_ = 0;
		cnt_ = 0;
		n_ = other.n_;
		delete[] container_;
		container_ = new T[n_];
		for (size_t i{0}; i < other.cnt_; ++i) push(other._at(i));
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
/*
	template<typename Tm>
	static BoundedBuffer<T, n> make_buffer(Tm && ele) {
		BoundedBuffer<T, n> buffer{};
		buffer.push(std::forward<Tm>(ele));
		return buffer;
	}

	template<typename... ELES>
	static BoundedBuffer<T, n> make_buffer(ELES && ...eles) {
		BoundedBuffer<T, n> buffer{};
		buffer.push_many(std::forward<decltype(eles)>(eles)...);
		return buffer;
	}
	*/
private:
	index_type index_{0};
	size_type cnt_{0};
	size_type n_{0};
	container_type container_;

	reference _at(index_type const & i) { return container_[i % n_]; }
	const_reference _at(index_type const & i) const { return container_[i % n_]; }

	void throwIfEmpty() const { if (empty()) throw std::logic_error{"empty container"}; }
	void throwIfFull() const { if (full()) throw std::logic_error{"full container"}; }

	index_type lastIndex() const noexcept { return index_ + cnt_ - 1; }
	index_type addToIndex() noexcept { return index_ + cnt_++; }
};

#endif /* SRC_BOUNDEDBUFFER_H_ */
