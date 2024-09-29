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

	Set* union_with(Set* other) {
		return new Set(
			[=](Object* o) {
				return in(o) || other->in(o);
			}
		);
	}

	Set* intersection_with(Set* other) {
		return new Set(
			[=](Object* o) {
				return in(o) && other->in(o);
			}
		);
	}

	Set* minus_with(Set* other) {
		return new Set(
			[=](Object* o) {
				return in(o) && !other->in(o);
			}
		);
	}

	Set* cartesian_with(Set* other) {
		Set* prod = new Set(
			[=](Object* o) {
				if ((o->type) != TUPLE) return false;
				Tuple* t = (Tuple*)o;
				// For canonical tuple result of the cartesian product
				if (t->length == 2) {
					Object* o1 = t->get(0);
					Object* o2 = t->get(1);
					return in(o1) && other->in(o2);
				}
				// For fully unwrapped common usage, (e.g. R^2xR gives ((a,b),c), but mostly used as (a,b,c))
				else if (t->length > 2) {
					for (size_t i = 0; i < auxiliaryMemory.size(); i++) {
						if (t->length <= i) return false;
						if (!((Set*)auxiliaryMemory[i])->in(t->get(i))) {
							return false;
						}
					}
					return true;
				}
			}
		);
		if (auxiliaryMemory.size() == 0) prod->auxiliaryMemory.push_back(this);
		else {
			for (size_t i = 0; i < auxiliaryMemory.size(); i++) {
				prod->auxiliaryMemory.push_back(auxiliaryMemory[i]);
			}
		}
		if (other->auxiliaryMemory.size() == 0) prod->auxiliaryMemory.push_back(other);
		else {
			for (size_t i = 0; i < other->auxiliaryMemory.size();i++) {
				prod->auxiliaryMemory.push_back(other->auxiliaryMemory[i]);
			}
		}
		return prod;
	}

};