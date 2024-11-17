#pragma once

#include <vector>
#include <string>
#include "Types.h"

struct AssosciatedMemory {
	std::vector<void*> auxiliaryMemory;
	std::vector<void*> ownedMemeory;
	~AssosciatedMemory() {
		for (void* p : ownedMemeory) delete p;
	}
};

// Base class for all mathematical objects
class Object {
public:
	AssosciatedMemory* asoc = nullptr;
	Type type = Types::UNKNOWN;

	~Object() {
		delete asoc;
	}
};