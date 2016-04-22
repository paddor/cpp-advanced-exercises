#ifndef SRC_BOUNDEDBUFFER_H_
#define SRC_BOUNDEDBUFFER_H_

#include <boost/operators.hpp>

#include <algorithm>
#include <memory>

#include <utility>
#include <stdexcept>

#include <iostream>

template <typename T>
struct BoundedBuffer {
	template<typename Container, typename Ref> struct BBIterator;

	using memory_type = std::unique_ptr<char[]>;
	using value_type = T;
	using reference = value_type &;
	using const_reference = value_type const &;
	using size_type = size_t;
	using iterator = BBIterator<BoundedBuffer, reference>;
	using const_iterator = BBIterator<const BoundedBuffer, const_reference>;

	BoundedBuffer(size_type n) : n_ { n }, container_ {newMemory()} {
		if (n <= 0) throw std::invalid_argument{"n must be > 0"};
	}
	BoundedBuffer(BoundedBuffer const & rhs) :
		index_{rhs.index_}, cnt_{rhs.cnt_}, n_{rhs.n_}, container_{newMemory()} {
			copyFromBufferToArray(rhs, elements());
	}

	BoundedBuffer(BoundedBuffer && rhs) :
		index_{std::move(rhs.index_)}, cnt_{std::move(rhs.cnt_)}, n_{std::move(rhs.n_)}, container_{std::move(rhs.container_)} {
			rhs.index_ = 0;
			rhs.cnt_ = 0;
			rhs.n_ = 0;
			rhs.container_ = nullptr;
	}

	BoundedBuffer & operator=(BoundedBuffer const & rhs) {
		if (&rhs == this) return *this;

		memory_type tmp{rhs.newMemory()};
		copyFromBufferToArray(rhs, elements(tmp));
		std::swap(container_, tmp);

		copyTrivialMembers(rhs);
		return *this;
	}

	BoundedBuffer & operator=(BoundedBuffer && rhs) {
		if (&rhs == this) return *this;

		std::swap(container_, rhs.container_);
		copyTrivialMembers(rhs);
		return *this;
	}

	~BoundedBuffer() { clear(); }

	bool empty() const noexcept { return cnt_ == 0; }
	bool full() const noexcept { return cnt_ == n_; }

	size_type size() const noexcept { return cnt_; }

	reference front() {
		throwIfEmpty();
		return at(0);
	}
	const_reference front() const {
		throwIfEmpty();
		return at(0);
	}

	reference back() {
		throwIfEmpty();
		return at(backIndexOffset());
	}
	const_reference back() const {
		throwIfEmpty();
		return at(backIndexOffset());
	}

	void push(T const & ele) {
		throwIfFull();
		::new(elements() + positionToAdd()) T{ele};
		++cnt_;
	}
	void push(T && ele) {
		throwIfFull();
		::new(elements() + positionToAdd()) T{std::move(ele)};
		++cnt_;
	}

	void pop() {
		throwIfEmpty();
		front().~T();
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

	iterator begin() { return iterator{this}; }
	iterator end() { return iterator{this, cnt_}; }

	const_iterator begin() const { return const_iterator{this}; }
	const_iterator end() const { return const_iterator{this, cnt_}; }

	const_iterator cbegin() const { return const_iterator{this}; }
	const_iterator cend() const { return const_iterator{this, cnt_}; }

	template<typename FIRST, typename... REST>
	void push_many(FIRST && first, REST && ...rest) {
		push(std::forward<FIRST>(first));
		push_many(std::forward<decltype(rest)>(rest)...);
	}

	template<typename FIRST>
	void push_many(FIRST && first) {
		push(std::forward<FIRST>(first));
	}

	reference at(size_type const i) { return elements()[calcMod(index_ + i)]; }
	const_reference at(size_type const i) const { return elements()[calcMod(index_ + i)]; }
	void clear() { while(!empty()) pop(); }
private:
	size_type index_{0};
	size_type cnt_{0};
	size_type n_{0};
	memory_type container_;


	void throwIfEmpty() const { if (empty()) throw std::logic_error{"empty container"}; }
	void throwIfFull() const { if (full()) throw std::logic_error{"full container"}; }

	void copyFromBufferToArray(BoundedBuffer const& from, T* to) {
		for (size_t i{0}; i < from.cnt_; ++i) {
			auto const pos = calcMod(from.index_ + i);
			to[pos] = from.elements()[pos];
		}
	}
	size_type calcMod(size_type const & i) const noexcept { return i % n_; }
	size_type backIndexOffset() const noexcept { return cnt_ - 1; }
	size_type positionToAdd() noexcept { return calcMod(index_ + cnt_); }
	char * newMemory() const { return new char[sizeof(T) * n_]; }
	T* elements() const { return elements(container_); }
	T* elements(memory_type const & container) const { return reinterpret_cast<T*>(container.get()); }
	void copyTrivialMembers(BoundedBuffer const & rhs) {
		index_ = rhs.index_;
		cnt_ = rhs.cnt_;
		n_ = rhs.n_;
	}
public:
	template<typename Container, typename Ref>
	struct BBIterator : public boost::random_access_iterator_helper<BBIterator<Container, Ref>, T> {
		using difference_type = std::ptrdiff_t;
		using index_type = long long;

	public:
		explicit BBIterator(Container * b, size_type i = 0) : buffer_{b}, index_{i} { }
		BBIterator(const BBIterator & rhs) = default;
		BBIterator & operator=(BBIterator const & rhs) = default;

		Ref operator*() const {
			this->checkEnd();
			return this->buffer_->at(this->index_);
		}

		BBIterator & operator++() {
			checkEnd();
			++index_;
			return *this;
		}
		BBIterator & operator--() {
			checkBegin();
			--index_;
			return *this;
		}
		BBIterator & operator+=(index_type const & rhs) {
			checkOverflow(index_, rhs);
			index_ += rhs;
			return *this;
		}
		BBIterator & operator+=(BBIterator const & rhs) { return *this += rhs.index_; }
		BBIterator & operator-=(index_type const & rhs) {
			checkUnderflow(index_, rhs);
			index_ -= rhs;
			return *this;
		}
		BBIterator & operator-=(BBIterator const & rhs) {
			checkSameBuffers(rhs);
			return *this -= rhs.index_;
		}

		bool operator<(BBIterator const & rhs) const {
			checkSameBuffers(rhs);
			return index_ < rhs.index_;
		}
		bool operator==(BBIterator const & rhs) const {
			return index_ ==  rhs.index_;
		}
		friend std::ostream & operator<<(std::ostream & out, const BBIterator & it) {
			out << "BBIterator{" << &it.buffer_ << ", " << it.index_ << "}";
			return out;
		}
		friend difference_type operator-(BBIterator const & lhs, BBIterator const & rhs) {
			lhs.checkSameBuffers(rhs);
			lhs.checkUnderflow(lhs.index_, rhs.index_);
			return lhs.index_ - rhs.index_;
		}

	private:
		Container * buffer_;
		size_type index_{0};

		void checkEnd() const { if (index_ == buffer_->cnt_) throw std::logic_error{"IT at end"}; }
		void checkBegin() const { if (index_ == 0) throw std::logic_error{"IT at beginning"}; }
		void checkSameBuffers(BBIterator const & rhs) const { if (buffer_ != rhs.buffer_) throw std::logic_error{"different buffer size"}; }

		void checkOverflow(index_type lhs, index_type rhs) const {
			if (lhs > 0 && rhs > static_cast<index_type>(buffer_->cnt_) - lhs) throw std::logic_error{"overflow detected"};
		}
		void checkUnderflow(index_type lhs, index_type rhs) const {
			if (lhs < rhs) throw std::logic_error{"underflow detected"};
		}
	};

};

#endif /* SRC_BOUNDEDBUFFER_H_ */
