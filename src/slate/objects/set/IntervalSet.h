#pragma once

#include "Set.h"
#include "../number/Number.h"

class IntervalSet : public Set {
public:

	IntervalSet(Number* lower, Number* higher, bool closedLow, bool closedHigh) : Set(
		[=](Object* o) {
			if (o->type == Types::INTERVAL) {
				// TODO: Add implementation
			}
			if (o->type == Types::NUMBER) {
				Number* n = (Number*)o;
				return *n > *lower && *n < *higher || (closedLow && *n == *lower) || (closedHigh && *n == *higher);
			}
			return true;
		}
	) {}

};