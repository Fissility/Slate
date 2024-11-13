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

	/*
	* @brief The union operation on two sets
	* @return Returns a set which is the union of the two
	*/
	Set* union_with(Set* other) {
		return new Set(
			[=](Object* o) {
				return in(o) || other->in(o);
			}
		);
	}

	/*
	* @brief Intersection operation on two sets
	* @return Returns a set which is the intersection of the two
	*/
	Set* intersection_with(Set* other) {
		return new Set(
			[=](Object* o) {
				return in(o) && other->in(o);
			}
		);
	}

	/*
	* @brief Minus operation between two sets
	* @return Returns a set which contains all elements in the first set that are not in the second set
	*/
	Set* minus_with(Set* other) {
		return new Set(
			[=](Object* o) {
				return in(o) && !other->in(o);
			}
		);
	}

	/*
	* @brief Cartesian operation between two sets
	* @return Returns the set which is the cartesian product of the two sets
	*/
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
			}
		);
		return prod;
	}

	std::string toString() {
		return "<SET>";
	}

};