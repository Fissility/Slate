#pragma once

#include <vector>
#include <string>
#include "objects/function/Function.h"
#include "objects/set/NSet.h"
#include "objects/set/ZSet.h"
#include "objects/set/QSet.h"
#include "objects/set/RSet.h"
#include "objects/set/USets.h"
#include "objects/set/IntervalSet.h"
#include "objects/function/BinaryOperator.h"

enum Presidences {
	ELEMENT_BINDING,
	ADDITION,
	SUBTRACTION = ADDITION,
	MULTIPLICATION,
	DIVISION = MULTIPLICATION,
	POWER,
	SET_BINDING,
	CARTESIAN
};

struct Definitions {
	// Map of objects and the names by which they are identified
	std::unordered_map <std::string, Object*> definitions;
	// Map of the printable strings of some objects
	std::unordered_map <Object*, std::string> stringValues;
	void registerDefinition(std::string& name, Object* o);
	bool objectExists(std::string& name);
	Object* getObject(std::string& name);

	void registerString(Object* o, std::string string);
	bool objectHasString(Object* o);
	std::string getString(Object* o);

	void registerBaseObject(Object* o, std::string name);

	void clear();
};

// Holds the set of symbols and default definitions for initialising a context
namespace SlateDefinitions {
	
	extern std::vector<std::string> symbolBases;
	extern std::vector<std::string> symbolFlares;
	extern std::vector<std::string> controlSeqeuenceCharacters;
	extern std::vector<std::string> binaryOperators;
	extern std::unordered_map<std::string, std::string> controlSequenceFunctions;

	extern void loadSymbols();
	extern Definitions buildDefaultDefinitions();

}