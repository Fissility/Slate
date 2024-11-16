#pragma once

#include "Set.h"

class QSet : public Set {
public:

	QSet() : Set(
		[](Object* o) {
			if (o->type != Types::INTERVAL && o->type != Types::NUMBER) return false;
			return true;
		}
	) {}

};


class QStarSet : public Set {
public:

	QStarSet() : Set(
		[](Object* o) {
			if (o->type == Types::NUMBER && *(Number*)o == 0) return false;
			if (o->type != Types::INTERVAL) return false;
			return true;
		}
	) {}

};