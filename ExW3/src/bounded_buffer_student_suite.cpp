#include "bounded_buffer_student_suite.h"
#include "cute.h"

#include "BoundedBuffer.h"

void testValueCtorWithLargeInput() {
	RingN<5> four{19};
	ASSERT_EQUAL((RingN<5>{4}), four);
}

void testIncrement() {
	RingN<5> two{1};
	++two;
	ASSERT_EQUAL((RingN<5>{2}), two);
}

void testIncrementOverflow() {
	RingN<5> zero{4};
	++zero;
	ASSERT_EQUAL((RingN<5>{0}), zero);
}

void testDecrement() {
	RingN<5> two{3};
	--two;
	ASSERT_EQUAL((RingN<5>{2}), two);
}

void testDecrementUnderflow() {
	RingN<5> four{0};
	--four;
	ASSERT_EQUAL((RingN<5>{4}), four);
}

void testMinus11Underflow() {
	RingN<5> two{3};
	two -= 11;
	ASSERT_EQUAL((RingN<5>{2}), two);
}

void testAddOverflow() {
	RingN<5> two{3};
	two += 4;
	ASSERT_EQUAL((RingN<5>{2}), two);
}

void testZeroN() {
	RingN<0> zero{2};
	ASSERT_EQUAL(RingN<0>{0}, zero);
}

void testZeroNIncrement() {
	RingN<0> zero{0};
	++zero;
	ASSERT_EQUAL(RingN<0>{0}, zero);
}

void testZeroNDecrement() {
	RingN<0> zero{0};
	--zero;
	ASSERT_EQUAL(RingN<0>{0}, zero);
}

cute::suite make_suite_bounded_buffer_student_suite() {
	cute::suite s;
	s.push_back(CUTE(testValueCtorWithLargeInput));
	s.push_back(CUTE(testIncrement));
	s.push_back(CUTE(testIncrementOverflow));
	s.push_back(CUTE(testDecrement));
	s.push_back(CUTE(testDecrementUnderflow));
	s.push_back(CUTE(testMinus11Underflow));
	s.push_back(CUTE(testAddOverflow));
	s.push_back(CUTE(testZeroN));
	s.push_back(CUTE(testZeroNIncrement));
	s.push_back(CUTE(testZeroNDecrement));
	return s;
}



