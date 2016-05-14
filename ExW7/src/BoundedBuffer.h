#ifndef SRC_BOUNDEDBUFFER_H_
#define SRC_BOUNDEDBUFFER_H_

#include <boost/operators.hpp>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

template <typename T>
struct BoundedBuffer {
	template<typename Container, typename Ref> struct BBIterator;

	using value_type = T;
	using reference = value_type &;
	using const_reference = value_type const &;
	using size_type = size_t;
	using iterator = BBIterator<BoundedBuffer, reference>;
	using const_iterator = BBIterator<const BoundedBuffer, const_reference>;
	using memory_type = std::unique_ptr<char[]>;

	explicit BoundedBuffer(size_type capacity) : capacity_ {capacity}, container_ {newMemory()} {
		if (capacity == 0) throw std::invalid_argument{"capacity must be > 0"};
	}

	~BoundedBuffer() { clear(); }

	BoundedBuffer(BoundedBuffer const & rhs) : capacity_{rhs.capacity_}, container_{newMemory()} {
		copyFromBoundedBuffer(rhs);
	}
	BoundedBuffer(BoundedBuffer && rhs) { swap(rhs); }

	BoundedBuffer & operator=(BoundedBuffer const & rhs) {
		if (&rhs == this) return *this;

		auto tmp{rhs};
		tmp.swap(*this);
		return *this;
	}
	BoundedBuffer & operator=(BoundedBuffer && rhs) {
		if (&rhs == this) return *this;

		swap(rhs);
		return *this;
	}

	bool empty() const noexcept { return size_ == 0; }
	bool full() const noexcept { return size_ == capacity_; }
	size_type size() const noexcept { return size_; }

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
		return at(backOffset());
	}
	const_reference back() const {
		throwIfEmpty();
		return at(backOffset());
	}

	void push(T const & ele) {
		throwIfFull();
		::new(elements() + positionToAdd()) T{ele};
		++size_;
	}
	void push(T && ele) {
		throwIfFull();
		::new(elements() + positionToAdd()) T{std::move(ele)};
		++size_;
	}

	void pop() {
		throwIfEmpty();
		front().~T();
		--size_;
		++index_;
	}

	void swap(BoundedBuffer & b) {
		using std::swap;
		swap(index_, b.index_);
		swap(size_, b.size_);
		swap(capacity_, b.capacity_);
		swap(container_, b.container_);
	}

	iterator begin() { return iterator{this}; }
	iterator end() { return iterator{this, size_}; }

	const_iterator begin() const { return const_iterator{this}; }
	const_iterator end() const { return const_iterator{this, size_}; }

	const_iterator cbegin() const { return const_iterator{this}; }
	const_iterator cend() const { return const_iterator{this, size_}; }

	template<typename... ELES>
	static BoundedBuffer<T> make_buffer(ELES && ...eles) {
		BoundedBuffer<T> buffer{sizeof...(ELES)};
		buffer.push_many(std::forward<decltype(eles)>(eles)...);
		return buffer;
	}
	template<typename HEAD, typename... TAIL>
	void push_many(HEAD && head, TAIL && ...tail) {
		push(std::forward<HEAD>(head));
		push_many(std::forward<decltype(tail)>(tail)...);
	}
	template<typename HEAD>
	void push_many(HEAD && head) { push(std::forward<HEAD>(head)); }

	reference at(size_type const i) { return elements()[calcMod(index_ + i)]; }
	const_reference at(size_type const i) const { return elements()[calcMod(index_ + i)]; }
	void clear() { while(!empty()) pop(); }

private:
	size_type index_{0};
	size_type size_{0};
	size_type capacity_{0};
	memory_type container_;


	void throwIfEmpty() const { if (empty()) throw std::logic_error{"empty container"}; }
	void throwIfFull() const { if (full()) throw std::logic_error{"full container"}; }

	size_type calcMod(size_type const & i) const noexcept { return i % capacity_; }
	size_type backOffset() const noexcept { return size_ - 1; }

	size_type positionToAdd() noexcept { return calcMod(index_ + size_); }

	void copyFromBoundedBuffer(BoundedBuffer const & rhs) { for (auto const & e : rhs) push(e); }

	char * newMemory() const { return new char[sizeof(T) * capacity_]; }
	T* elements() const { return elements(container_); }
	T* elements(memory_type const & container) const { return reinterpret_cast<T*>(container.get()); }

public:
	template<typename Container, typename Ref>
	struct BBIterator : public boost::random_access_iterator_helper<BBIterator<Container, Ref>, T> {
		using difference_type = std::ptrdiff_t;
		using index_type = long long; // hmm narrowing problems with [-1] test.. is there a better solution?
	public:
		explicit BBIterator(Container * const b, size_type const p = 0) : buffer_{b}, pos_{p} { }
		BBIterator(const BBIterator & rhs) = default;
		BBIterator & operator=(BBIterator const & rhs) = default;

		Ref operator*() const {
			throwIfEnd();
			return buffer_->at(pos_);
		}

		BBIterator & operator++() {
			throwIfEnd();
			++pos_;
			return *this;
		}
		BBIterator & operator--() {
			throwIfBegin();
			--pos_;
			return *this;
		}
		BBIterator & operator+=(index_type const & rhs) {
			checkOverflow(pos_, rhs);
			pos_ += rhs;
			return *this;
		}
		BBIterator & operator+=(BBIterator const & rhs) {
			checkSameBuffers(rhs);
			return *this += rhs.pos_;
		}
		BBIterator & operator-=(index_type const & rhs) {
			checkUnderflow(pos_, rhs);
			pos_ -= rhs;
			return *this;
		}
		BBIterator & operator-=(BBIterator const & rhs) {
			checkSameBuffers(rhs);
			return *this -= rhs.pos_;
		}

		bool operator<(BBIterator const & rhs) const {
			checkSameBuffers(rhs);
			return pos_ < rhs.pos_;
		}
		bool operator==(BBIterator const & rhs) const {
			checkSameBuffers(rhs);
			return pos_ ==  rhs.pos_;
		}
		friend std::ostream & operator<<(std::ostream & out, BBIterator const & it) {
			out << "BBIterator{" << &it.buffer_ << ", " << it.pos_ << "}";
			return out;
		}
		friend difference_type operator-(BBIterator const & lhs, BBIterator const & rhs) {
			lhs.checkSameBuffers(rhs);
			lhs.checkUnderflow(lhs.pos_, rhs.pos_);
			return lhs.pos_ - rhs.pos_;
		}

	private:
		Container * const buffer_;
		size_type pos_{0};

		void throwIfEnd() const { if (pos_ == buffer_->size_) throw std::logic_error{"IT at end"}; }
		void throwIfBegin() const { if (pos_ == 0) throw std::logic_error{"IT at beginning"}; }
		void checkSameBuffers(BBIterator const & rhs) const {
			if (buffer_ != rhs.buffer_) throw std::logic_error{"different buffers"};
		}

		void checkOverflow(index_type const & lhs, index_type const & rhs) const {
			if (lhs > 0 && rhs > static_cast<index_type>(buffer_->size_) - lhs) throw std::logic_error{"overflow detected"};
		}
		void checkUnderflow(index_type const & lhs, index_type const & rhs) const {
			if (lhs < rhs) throw std::logic_error{"underflow detected"};
		}
	};
};

#endif /* SRC_BOUNDEDBUFFER_H_ */
