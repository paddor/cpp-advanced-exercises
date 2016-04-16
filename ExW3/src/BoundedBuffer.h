#ifndef SRC_BOUNDEDBUFFER_H_
#define SRC_BOUNDEDBUFFER_H_

#include <boost/operators.hpp>

#include <array>
#include <utility>
#include <stdexcept>

template <size_t N, typename size_type=size_t>
struct RingN :
		private boost::equality_comparable<RingN<N, size_type>>,
		private boost::addable<RingN<N, size_type>>,
		private boost::subtractable<RingN<N, size_type>>,
		private boost::incrementable<RingN<N, size_type>> {
	RingN(size_type x=0ul) : val_{static_cast<size_type>(x % N)} { }
	size_type operator*() const { return val_; }
	bool operator==(RingN const & r) const { return val_ == r.val_; }
	RingN operator+=(RingN const & r) {
		val_ = (val_ + r.val_) % N;
		return *this;
	}
	RingN operator-=(RingN const & r) {
		if (val_ < r.val_) val_ = N + (r.val_ % N) - 1;
		val_ = (val_ - r.val_) % N;
		return *this;
	}
	friend RingN operator++(RingN & r) { return r += 1; }
	friend RingN operator--(RingN & r) { return r -= 1; }
private:
	size_type val_;
};

template <typename size_type>
struct RingN<0, size_type> :
		private boost::equality_comparable<RingN<0, size_type>>,
		private boost::addable<RingN<0, size_type>>,
		private boost::subtractable<RingN<0, size_type>>,
		private boost::incrementable<RingN<0, size_type>> {
	RingN(size_type x = 0ul) { }
	size_type operator*() const { return 0; }
	bool operator==(RingN const & r) const { return 0 == *r; }
	RingN operator+=(RingN const & r) { return *this; }
	RingN operator-=(RingN const & r) { return *this; }
	friend RingN operator++(RingN & r) { return r; }
	friend RingN operator--(RingN & r) { return r; }
private:
};


template <typename T, size_t N>
struct BoundedBuffer {
	using container_type = std::array<T, N>;
	using value_type = typename container_type::value_type;
	using reference = typename container_type::reference;
	using const_reference = typename container_type::const_reference;
	using size_type = typename container_type::size_type;
	using index_type = RingN<N, size_type>;

	BoundedBuffer() = default;
	BoundedBuffer(BoundedBuffer const & other) :
		container{other.container}, index{other.index}, count{other.count} { }

	BoundedBuffer(BoundedBuffer && other) :
		container{std::move(other.container)}, index{std::move(other.index)}, count{std::move(other.count)} { }

	BoundedBuffer & operator=(BoundedBuffer const & other) {
		container = other.container;
		index = other.index;
		count = other.count;
		return *this;
	}

	BoundedBuffer & operator=(BoundedBuffer && other) {
		container = std::move(other.container);
		index = std::move(other.index);
		count = std::move(other.count);
		return *this;
	}

	bool empty() const noexcept { return count == 0; }
	bool full() const noexcept { return count == N; }
	size_type size() const noexcept { return count; }

	reference front() {
		throwIfEmpty();
		return _at(index);
	}
	const_reference front() const {
		throwIfEmpty();
		return _at(index);
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
		--count;
		++index;
	}
	void swap(BoundedBuffer & b) {
		if (!(empty() && b.empty())) {
			using std::swap;
			swap(container, b.container);
			swap(index, b.index);
			swap(count, b.count);
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

	template<typename Tm>
	static BoundedBuffer<T, N> make_buffer(Tm && ele) {
		BoundedBuffer<T, N> buffer{};
		buffer.push(std::forward<Tm>(ele));
		return buffer;
	}

	template<typename... ELES>
	static BoundedBuffer<T, N> make_buffer(ELES && ...eles) {
		BoundedBuffer<T, N> buffer{};
		buffer.push_many(std::forward<decltype(eles)>(eles)...);
		return buffer;
	}
private:
	container_type container{};
	index_type index{0};
	size_type count{0};

	reference _at(index_type const & i) { return container.at(*i); }
	const_reference _at(index_type const & i) const { return container.at(*i); }

	void throwIfEmpty() const { if (empty()) throw std::logic_error{"empty container"}; }
	void throwIfFull() const { if (full()) throw std::logic_error{"full container"}; }

	index_type lastIndex() const noexcept { return index + count - 1; }
	index_type addToIndex() noexcept { return index + count++; }
};

#endif /* SRC_BOUNDEDBUFFER_H_ */
