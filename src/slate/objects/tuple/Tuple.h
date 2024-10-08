#pragma once

#include "../Object.h"

class Tuple : public Object {

public:

	size_t length;
	Object** objects;

	Tuple(size_t length) {
		this->type = Types::TUPLE;
		this->length = length;
		objects = new Object*[length];
	}

	// The memory passed as the array should be on the heap and it will be owned by the tuple object
	Tuple(size_t length,Object* os []) {
		this->type = Types::TUPLE;
		this->length = length;
		objects = os;
	}

	~Tuple() {
		delete[] objects;
	}

	Object*& operator[](size_t i) {
		return objects[i];
	}

	Object* get(size_t index) {
		return objects[index];
	}

};