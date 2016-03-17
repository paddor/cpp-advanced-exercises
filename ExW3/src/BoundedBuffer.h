#ifndef SRC_BOUNDEDBUFFER_H_
#define SRC_BOUNDEDBUFFER_H_

#include <boost/operators.hpp>
#include <stdexcept>

template <size_t N>
struct RingN : boost::equality_comparable<RingN<N>>, boost::addable<RingN<N>>, boost::subtractable<RingN<N>>, boost::incrementable<RingN<N>> {
	RingN(size_t x = 0u) : val_{x % N} { }
	size_t operator*() const { return val_; }
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
	friend auto operator++(RingN<N> & r) {
		return r += 1;
	}
	friend auto operator--(RingN<N> & r) {
		return r -= 1;
	}
private:
	size_t val_;
};

template <>
struct RingN<0> : boost::equality_comparable<RingN<0>>, boost::addable<RingN<0>>, boost::subtractable<RingN<0>>, boost::incrementable<RingN<0>> {
	RingN(size_t x = 0u) : val_{0} { }
	size_t operator*() const { return val_; }
	bool operator==(RingN const & r) const { return val_ == r.val_; }
	RingN operator+=(RingN const & r) { return *this; }
	RingN operator-=(RingN const & r) { return *this; }
	friend auto operator++(RingN & r) { return r; }
	friend auto operator--(RingN & r) { return r; }
private:
	size_t val_;
};


template <typename T, size_t N>
struct BoundedBuffer {
	using Container = std::array<T, N>;
	using container_type = Container;
	using value_type = typename Container::value_type;
	using reference = typename Container::reference;
	using const_reference = typename Container::const_reference;
	using size_type = typename Container::size_type;

	using index_type = RingN<N>;

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
	bool full() const noexcept { return count >= N; }
	size_t size() const noexcept { return count; }

	reference front() {
		check_empty();
		return _at(index);
	}
	const_reference front() const {
		check_empty();
		return _at(index);
	}

	reference back() {
		check_empty();
		return _at(index + count - 1);
	}
	const_reference back() const {
		check_empty();
		return _at(index + count - 1);
	}

	void push(T const & ele) {
		check_full();
		_at(index + count++) = ele;
	}
	void push(T && ele) {
		check_full();
		_at(index + count++) = std::move(ele);
	}

	void pop() {
		check_empty();
		--count;
		++index;
	}
	void swap(BoundedBuffer & b) {
		if (!(empty() && b.empty())) {
			std::swap(container, b.container);
			std::swap(index, b.index);
			std::swap(count, b.count);
		}
	}
private:
	Container container{};
	index_type index{0};
	size_t count{0};

	reference _at(index_type i) { return container.at(*i); }
	const_reference _at(index_type i) const { return container.at(*i); }

	reference _at(size_t i) { return _at(index_type{i}); }
	const_reference _at(size_t i) const { return _at(index_type{i}); }

	void check_empty() const { if (empty()) throw std::logic_error{"empty container"}; }
	void check_full() const { if (full()) throw std::logic_error{"full container"}; }

};

#endif /* SRC_BOUNDEDBUFFER_H_ */
