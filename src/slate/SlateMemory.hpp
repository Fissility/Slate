#pragma once

#include <list>
#include <unordered_map>
#include "objects/Object.h"

namespace SlateMemory {

	extern std::unordered_map<Object*, std::vector<Object*>> ownershipMap;
	
	extern void freeMem(Object* p);
	extern void attach(Object* parent, Object* child);

};

