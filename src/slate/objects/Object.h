#pragma once

#include <vector>

enum Types {
	UNKNOWN,
	NUMBER,
	INTERVAL,
	SET,
	FUNCTION,
	BINARY_OPERATOR,
	TUPLE,
	CATEGORY
};

typedef size_t Type;

class Object {
public:
	std::vector<void*> auxiliaryMemory;
	std::vector<void*> ownedMemeory;
	Type type = Types::UNKNOWN;

	~Object() {
		for (void* m : ownedMemeory) {
			delete m;
		}
	}
};