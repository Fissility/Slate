#pragma once

#include "../Object.h"

class Tuple : public Object {

private:
  Object **objects = nullptr;

public:
  size_t length = 0;

  Tuple(size_t length, Object *os[]) {
    this->type = Types::TUPLE;
    this->length = length;
    this->objects = os;
  }

  Object *get(size_t index) { return objects[index]; }

  Object *&operator[](size_t i) { return objects[i]; }

  ~Tuple() { delete[] objects; }
};
