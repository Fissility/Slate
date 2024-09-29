#pragma once

#include "../Object.h"

class Tuple : public Object{

public:

	size_t length;
	Object** objects;

	Tuple(size_t length,Object* os []) {
		this->type = Types::TUPLE;
		this->length = length;
		objects = new Object*[length];
		for (size_t i = 0; i < length; i++) {
			objects[i] = os[i];
		}
	}

	~Tuple() {
		delete[] objects;
	}

	Object* get(size_t index) {
		return objects[index];
	}

};