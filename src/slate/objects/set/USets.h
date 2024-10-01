#pragma once

#include "Set.h"

// I dont even want to hear it
// Set of all sets, for practical purpouses this is needed
class AllSetsSet : public Set {
public:

	AllSetsSet() : Set(
		[](Object* o) {
			if (o->type == Types::SET) return true;
			return false;
		}
	) {}

};

class USet : public Set {
public:

	USet() : Set(
		[](Object* o) {
			return true;
		}
	) {}

};