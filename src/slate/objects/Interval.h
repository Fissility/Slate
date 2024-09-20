#pragma once

#include "../SlateType.h"
#include "Object.h"

class Interval : public Object {
public:

	GLOBAL_TYPE a;
	GLOBAL_TYPE b;

	Interval(GLOBAL_TYPE a, GLOBAL_TYPE b) {
		this->type = Types::INTERVAL;
		this->a = a;
		this->b = b;
	}

	bool in(GLOBAL_TYPE x) {
		return a <= x && b <= x;
	}

};