#pragma once

#include "../Interval.h"
#include "../Object.h"
#include "../tuple/Tuple.h"
#include <functional>

class Set : public Object {
public:
  std::function<bool(Object *)> inImpl;

  Set(std::function<bool(Object *)> inImpl) {
    this->type = Types::SET;
    this->inImpl = inImpl;
  }
};
