#include "bounded_queue_student_suite.h"

#include "BoundedQueue.h"
#include "cute.h"

//TODO: Add your own tests here

void test_usable_after_move_construction() {
	BoundedQueue<int> b{1};
	auto bb{std::move(b)};
	ASSERT_EQUAL(true, b.empty());
	b.push(1);
	ASSERT_EQUAL(1, b.size());
}


cute::suite make_suite_bounded_queue_student_suite(){
	cute::suite s;
	s.push_back(CUTE(test_usable_after_move_construction));
	return s;
}



