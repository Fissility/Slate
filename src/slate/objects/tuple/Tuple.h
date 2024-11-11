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

	std::string toString() {
		std::string out = "(";
		for (size_t i = 0; i < length; i++) {
			out += objects[i]->toString();
			if (i != length - 1) out += ",";
		}
		out += ")";
		return out;
	}

};