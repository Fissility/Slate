#pragma once

#include "../Object.h"
#include <format>

class Tuple : public Object {

public:

	size_t length;
	Object** objects;

	Tuple(size_t length,Object* os []) {
		this->type = Types::TUPLE;
		this->length = length;
		objects = os;
	}

	Object*& operator[](size_t i) {
		return objects[i];
	}

	Object* get(size_t index) {
		return objects[index];
	}

};