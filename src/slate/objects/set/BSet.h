#pragma once

#include "Set.h"
#include "../number/Number.h"

class BSet : public Set {
public:

	BSet() : Set(
		[](Object* o) {
			if (o->type != Types::NUMBER) return false;
			return *((Number*)o) == 1 || *((Number*)o) == 0;
		}
	) {}

};