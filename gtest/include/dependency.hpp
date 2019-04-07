#pragma once
#include <memory>
#include <common.hpp>

class DependA {};
class DependB {};

typedef std::shared_ptr<DependA> DependAPtr_t;

