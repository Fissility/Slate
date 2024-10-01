#pragma once

#include "Tuple.h"

class BiCategory : public Tuple {

public:

	BiCategory(Object* first, Object* second) : Tuple(2, new Object* [] {first, second}) {
		this->type = Types::CATEGORY;
	}

};