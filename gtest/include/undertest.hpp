#pragma once
#include <common.hpp>
#include <dependency.hpp>

class UnderTest {
  public:
    UnderTest(DependAPtr_t&);
    int operation1(DependB&);

  private:
    DependAPtr_t m_a;
};

