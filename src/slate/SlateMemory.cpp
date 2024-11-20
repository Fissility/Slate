#include "SlateMemory.hpp"

std::unordered_map<Object*, std::vector<Object*>> SlateMemory::ownershipMap;

void SlateMemory::attach(Object* p, Object* c) {
	if (ownershipMap.find(p) == ownershipMap.end()) ownershipMap[p] = std::vector<Object*>();
	ownershipMap[p].push_back(c);
}

void SlateMemory::freeMem(Object* p) {
	if (ownershipMap.find(p) != ownershipMap.end()) {
		for (Object* c : ownershipMap[p]) freeMem(c);
		ownershipMap.erase(p);
	}
	delete p;
}
