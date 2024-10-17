#pragma once

#include "Object.h"

class ObjectReference {

public:

	Object** ref;

	ObjectReference(Object*& ref) {
		this->ref = &ref;
	}

	operator Object* () {
		return *ref;
	}

	ObjectReference operator = (Object*& o) {
		return ObjectReference(o);
	}

};