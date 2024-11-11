#pragma once

#include "../../SlateType.h"
#include "../Object.h"
class Number : public Object {

public:
	GLOBAL_TYPE value;

	Number() {
		this->value = (GLOBAL_TYPE)0;
		this->type = Types::NUMBER;
	}

	Number(GLOBAL_TYPE value) {
		this->value = value;
		this->type = Types::NUMBER;
	}

	operator GLOBAL_TYPE() const {
		return value;
	}

	Number operator = (GLOBAL_TYPE& value) {
		return Number(value);
	}

	std::string toString() {
		return std::to_string(value);
	}

};