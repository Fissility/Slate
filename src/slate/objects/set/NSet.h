#pragma once

#include "Set.h"
#include "../number/Number.h"
#include <cmath>

class NSet : public Set {

public:

	NSet() : Set(
		[](Object* o) {
			switch (o->type) {
			case Types::INTERVAL: {
				Interval* i = (Interval*)o;
				if (i->b < 0) return false;
				return std::floor(i->a) != std::floor(i->b) || std::floor(i->a) == i->a || std::floor(i->b) == i->b;
			}
			case Types::NUMBER: {
				Number* n = (Number*)o;
				if (*n < 0) return false;
				return *n == std::floor(*n);
			}
			default:
				return false;
			}
		}
	) {}

};