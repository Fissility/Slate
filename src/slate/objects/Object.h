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
	CATEGORY,
	EXPRESSION
};

typedef size_t Type;

struct AssosciatedMemory {
	std::vector<void*> auxiliaryMemory;
	std::vector<void*> ownedMemeory;
	~AssosciatedMemory() {
		for (void* p : ownedMemeory) delete p;
	}
};

class Object {
public:
	AssosciatedMemory* asoc = nullptr;
	Type type = Types::UNKNOWN;

	~Object() {
		delete asoc;
	}
};