#include <undertest.hpp>

UnderTest::UnderTest(DependAPtr_t& a) 
  : m_a(a) {}

int UnderTest::operation1(DependB& b) {
  return 0;
}

