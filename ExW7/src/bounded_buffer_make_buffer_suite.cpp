#include "bounded_buffer_make_buffer_suite.h"
#include "cute.h"
#include "BoundedBuffer.h"

#include "MemoryOperationCounter.h"

void test_make_bounded_buffer_from_rvalue_argument_contains_one_element() {
	BoundedBuffer<MemoryOperationCounter> buffer = BoundedBuffer<MemoryOperationCounter>::make_buffer(MemoryOperationCounter{});
	ASSERT_EQUAL(1, buffer.size());
}

void test_make_bounded_buffer_from_rvalue_argument_object_moved() {
	MemoryOperationCounter expected{1, 0, true};
	BoundedBuffer<MemoryOperationCounter> buffer = BoundedBuffer<MemoryOperationCounter>::make_buffer(MemoryOperationCounter{});
	ASSERT_EQUAL(expected, buffer.front());
}

void test_bounded_buffer_constructed_with_lvalue_argument_object_copied() {
	MemoryOperationCounter expected{0, 1, true};
	MemoryOperationCounter insertee{};
	BoundedBuffer<MemoryOperationCounter> buffer = BoundedBuffer<MemoryOperationCounter>::make_buffer(insertee);
	ASSERT_EQUAL(expected, buffer.front());
}

void test_bounded_buffer_constructed_with_const_lvalue_argument_object_copied() {
	MemoryOperationCounter expected{0, 1, true};
	MemoryOperationCounter const insertee{};
	BoundedBuffer<MemoryOperationCounter> buffer = BoundedBuffer<MemoryOperationCounter>::make_buffer(insertee);
	ASSERT_EQUAL(expected, buffer.front());
}

void test_make_bounded_buffer_from_two_rvalue_arguments_contains_two_elements() {
	BoundedBuffer<MemoryOperationCounter> buffer = BoundedBuffer<MemoryOperationCounter>::make_buffer(MemoryOperationCounter{}, MemoryOperationCounter{});
	ASSERT_EQUAL(2, buffer.size());
}

void test_make_bounded_buffer_from_two_lvalue_arguments_contains_two_elements() {
	MemoryOperationCounter element1{}, element2{};
	BoundedBuffer<MemoryOperationCounter> buffer = BoundedBuffer<MemoryOperationCounter>::make_buffer(element1, element2);
	ASSERT_EQUAL(2, buffer.size());
}

void test_make_bounded_buffer_from_two_rvalue_arguments_first_element_moved() {
	MemoryOperationCounter expected{1, 0, true};
	BoundedBuffer<MemoryOperationCounter> buffer = BoundedBuffer<MemoryOperationCounter>::make_buffer(MemoryOperationCounter{}, MemoryOperationCounter{});

	ASSERT_EQUAL(expected, buffer.front());
}

void test_make_bounded_buffer_from_two_rvalue_arguments_second_element_moved() {
	MemoryOperationCounter expected{1, 0, true};
	BoundedBuffer<MemoryOperationCounter> buffer = BoundedBuffer<MemoryOperationCounter>::make_buffer(MemoryOperationCounter{}, MemoryOperationCounter{});
	ASSERT_EQUAL(expected, buffer.back());
}

void test_make_bounded_buffer_from_two_rvalue_arguments_first_element_copied() {
	MemoryOperationCounter expected{0, 1, true};
	MemoryOperationCounter lvalue{};
	BoundedBuffer<MemoryOperationCounter> buffer = BoundedBuffer<MemoryOperationCounter>::make_buffer(lvalue, MemoryOperationCounter{});
	ASSERT_EQUAL(expected, buffer.front());
}

void test_make_bounded_buffer_from_two_mixed_arguments_second_element_moved() {
	MemoryOperationCounter expected{1, 0, true};
	MemoryOperationCounter lvalue{};
	BoundedBuffer<MemoryOperationCounter> buffer = BoundedBuffer<MemoryOperationCounter>::make_buffer(lvalue, MemoryOperationCounter{});
	ASSERT_EQUAL(expected, buffer.back());
}

void test_make_bounded_buffer_from_two_rvalue_arguments_second_element_copied() {
	MemoryOperationCounter expected{0, 1, true};
	MemoryOperationCounter lvalue{};
	BoundedBuffer<MemoryOperationCounter> buffer = BoundedBuffer<MemoryOperationCounter>::make_buffer(MemoryOperationCounter{}, lvalue);
	ASSERT_EQUAL(expected, buffer.back());
}

void test_make_bounded_buffer_from_two_mixed_arguments_first_element_moved() {
	MemoryOperationCounter expected{1, 0, true};
	MemoryOperationCounter lvalue{};
	BoundedBuffer<MemoryOperationCounter> buffer = BoundedBuffer<MemoryOperationCounter>::make_buffer(MemoryOperationCounter{}, lvalue);
	ASSERT_EQUAL(expected, buffer.front());
}

cute::suite make_suite_bounded_buffer_make_buffer_suite() {
	cute::suite s;
	s.push_back(CUTE(test_make_bounded_buffer_from_rvalue_argument_contains_one_element));
   	s.push_back(CUTE(test_make_bounded_buffer_from_rvalue_argument_object_moved));
	s.push_back(CUTE(test_bounded_buffer_constructed_with_lvalue_argument_object_copied));
	s.push_back(CUTE(test_bounded_buffer_constructed_with_const_lvalue_argument_object_copied));
	s.push_back(CUTE(test_make_bounded_buffer_from_two_rvalue_arguments_contains_two_elements));
	s.push_back(CUTE(test_make_bounded_buffer_from_two_lvalue_arguments_contains_two_elements));
	s.push_back(CUTE(test_make_bounded_buffer_from_two_rvalue_arguments_first_element_moved));
	s.push_back(CUTE(test_make_bounded_buffer_from_two_rvalue_arguments_second_element_moved));
	s.push_back(CUTE(test_make_bounded_buffer_from_two_rvalue_arguments_first_element_copied));
	s.push_back(CUTE(test_make_bounded_buffer_from_two_mixed_arguments_second_element_moved));
	s.push_back(CUTE(test_make_bounded_buffer_from_two_rvalue_arguments_second_element_copied));
	s.push_back(CUTE(test_make_bounded_buffer_from_two_mixed_arguments_first_element_moved));
	return s;
}

