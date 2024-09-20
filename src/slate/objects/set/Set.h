#pragma once

#include "../Object.h"
#include "../Interval.h"
#include "../tuple/Tuple.h"
#include <functional>

class Set : public Object {
public:

	std::function<bool(Object*)> inImpl;

	Set(std::function<bool(Object*)> inImpl) {
		this->type = SET;
		this->inImpl = inImpl;
	}

	bool in(Object* o) {
		return inImpl(o);
	}

	Set* cartesian(Set* other) {
		return new Set(
			[=](Object* o) {
				if ((o->type) != TUPLE) return false;
				Tuple* t = (Tuple*)o;
				Object* o1 = t->get(0);
				Object* o2 = t->get(1);
				return in(o1) && other->in(o2);
			}
		);
	}

};