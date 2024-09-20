#pragma once

#include <vector>

enum Types {
	UNKNOWN,
	NUMBER,
	INTERVAL,
	SET,
	FUNCTION,
	TUPLE
};

typedef size_t Type;

class Object {
public:
	std::vector<void*> auxiliaryMemory;
	Type type = Types::UNKNOWN;
	~Object() {
		for (void* i : auxiliaryMemory) delete i;
	}
};