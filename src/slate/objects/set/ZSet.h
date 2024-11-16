#pragma once

#include "Set.h"
#include "../number/Number.h"
#include <cmath>

class ZSet : public Set {
public:

	ZSet() : Set(
		[](Object* o) {
			switch (o->type) {
			case Types::INTERVAL: {
				Interval* i = (Interval*)o;
				return std::floor(i->a) != std::floor(i->b) || std::floor(i->a) == i->a || std::floor(i->b) == i->b;
			}
			case Types::NUMBER: {
				Number* n = (Number*)o;
				return *n == std::floor(*n);
			}
			default:
				return false;
			}
		}
	){}
};

class ZStarSet : public Set {
public:

	ZStarSet() : Set(
		[](Object* o) {
			switch (o->type) {
			case Types::INTERVAL: {
				Interval* i = (Interval*)o;
				return std::floor(i->a) != std::floor(i->b) || std::floor(i->a) == i->a || std::floor(i->b) == i->b;
			}
			case Types::NUMBER: {
				Number* n = (Number*)o;
				return *n == std::floor(*n) && *n != 0;
			}
			default:
				return false;
			}
		}
	) {}
};