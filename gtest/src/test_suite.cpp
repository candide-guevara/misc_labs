#include <memory>
#include <gtest/gtest.h>
#include <undertest.hpp>

using namespace std;

TEST(mysuite, first) {
  auto dependA = make_shared<DependA>();
  UnderTest blackbox(dependA);
}

