#ifndef SRC_BOUNDEDQUEUE_H_
#define SRC_BOUNDEDQUEUE_H_

/*
 * 1. Why can't be use the front() and back() member functions from BoundedBuffer?
 * => Both return a reference to the element. Candidate for race conditions.
 * => Non atomic command and query (see below)
 * 2. Why doesn't it make sense to provide iterators for BoundedQueue?
 * => Iterators should lock the queue while they are "alive", highly inefficient.
 *	Can you suggest an alternative means for observing the BoundedQueue content?
 *	=> for_each as a locking member function
 *	=> const iterators with readlock while alive (could lead to long lasting readlocks and starving writelocks?)
 * 3. Why is pop() returning a value by value and not void as in BoundedBuffer?
 * Violates "command-query separation", but both steps need to be done "atomically".
 * If command and query is separated, the caller has to lock the queue.
 */

#include <condition_variable>
#include <chrono>
#include <memory>
#include <mutex>
#include <utility>

template <typename T, typename M=std::mutex, typename CV=std::condition_variable>
struct BoundedQueue {
	using guard = std::lock_guard<M>;
	using lock = std::unique_lock<M>;

	using value_type = T;
	using reference = value_type &;
	using const_reference = value_type const &;
	using size_type = size_t;
	using memory_type = std::unique_ptr<char[]>;

	explicit BoundedQueue(size_type capacity) : capacity_{capacity}, container_{newMemory()} {
		if (!capacity) throw std::invalid_argument{"capacity must be > 0"};
	}

	~BoundedQueue() { while (!_empty()) _pop(); }

	BoundedQueue(BoundedQueue const & rhs) : capacity_{rhs.capacity_}, container_{newMemory()} {
		guard lk{rhs.mx_};
		for (size_type i{0}; i < rhs._size(); ++i) _push(rhs._at(i));
	}
	BoundedQueue(BoundedQueue && rhs) : BoundedQueue{rhs.capacity_} { swap(rhs); }

	BoundedQueue & operator=(BoundedQueue const & rhs) {
		if (&rhs == this) return *this;

		auto tmp{rhs};
		swap(tmp);
		return *this;
	}
	BoundedQueue & operator=(BoundedQueue && rhs) {
		if (&rhs == this) return *this;

		swap(rhs);
		return *this;
	}

	bool empty() const noexcept { guard lk{mx_}; return _empty(); }
	bool full() const noexcept { guard lk{mx_}; return _full(); }
	size_type size() const noexcept  { guard lk{mx_}; return _size(); }

	void push(value_type const & ele) {
		lock lk{mx_};
		notFull_.wait(lk, [this]{ return !_full(); });

		_pushNotify(ele);
	}
	void push(value_type && ele) {
		lock lk{mx_};
		notFull_.wait(lk, [this]{ return !_full(); });

		new(pushBuffer()) value_type{std::move(ele)};
		++size_;
		notEmpty_.notify_one();
	}
	bool try_push(value_type const & ele) {
		guard lk{mx_};
		if (_full()) return false;

		_pushNotify(ele);
		return true;
	}
	template<class Rep, class Period>
	bool try_push_for(T const & ele, std::chrono::duration<Rep, Period> const & timeout) {
		lock lk{mx_};
		if (notFull_.wait_for(lk, timeout, [this]{ return !_full(); })) {
			_pushNotify(ele);
			return true;
		}
		return false;
	}

	value_type pop() {
		lock lk{mx_};
		notEmpty_.wait(lk, [this]{ return !_empty(); });

		value_type front = std::move(_at(0));
		_popNotify();
		return front;
	}
	bool try_pop(value_type & ele) {
		guard lk{mx_};
		if (_empty()) return false;

		_popNotify(ele);
		return true;
	}
	template<class Rep, class Period>
	bool try_pop_for(value_type & ele, std::chrono::duration<Rep,Period> const & timeout) {
		lock lk{mx_};
		if (notEmpty_.wait_for(lk, timeout, [this]{ return !_empty(); })) {
			_popNotify(ele);
			return true;
		}
		return false;
	}

	void swap(BoundedQueue & rhs) {
		if (this == &rhs) return;

		lock lk{mx_, std::defer_lock};
		lock lkRhs{rhs.mx_, std::defer_lock};
		std::lock(lk, lkRhs);

		using std::swap;
		swap(index_, rhs.index_);
		swap(size_, rhs.size_);
		swap(capacity_, rhs.capacity_);
		swap(container_, rhs.container_);
	}
private:
	mutable M mx_{};
	CV notEmpty_{};
	CV notFull_{};

	size_type index_{0};
	size_type size_{0};
	size_type capacity_{0};
	memory_type container_{};

	bool _empty() const noexcept { return !size_; }
	bool _full() const noexcept { return size_ == capacity_; }
	size_type _size() const noexcept { return size_; }

	void _push(value_type const & ele) {
		new(pushBuffer()) value_type{ele};
		++size_;
	}
	void _pushNotify(value_type const & ele) {
		_push(ele);
		notEmpty_.notify_one();
	}

	void _pop() {
		_at(0).~value_type();
		--size_;
		++index_;
	}
	void _popNotify() {
		_pop();
		notFull_.notify_one();
	}
	void _popNotify(value_type & ele) {
		ele = std::move(_at(0));
		_popNotify();
	}

	size_type calcMod(size_type const & i) const noexcept { return i % capacity_; }

	char * newMemory() const { return new char[sizeof(value_type) * capacity_]; }
	value_type * elements() const { return reinterpret_cast<value_type*>(container_.get()); }
	value_type * pushBuffer() { return elements() + calcMod(index_ + size_); }

	reference _at(size_type const i) { return elements()[calcMod(index_ + i)]; }
	const_reference _at(size_type const i) const { return elements()[calcMod(index_ + i)]; }
};

#endif /* SRC_BOUNDEDQUEUE_H_ */

