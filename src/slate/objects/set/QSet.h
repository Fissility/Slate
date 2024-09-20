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