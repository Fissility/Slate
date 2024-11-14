#pragma once

#include <vector>
#include <string>
#include "objects/function/Function.h"
#include "objects/set/NSet.h"
#include "objects/set/ZSet.h"
#include "objects/set/QSet.h"
#include "objects/set/RSet.h"
#include "objects/set/USets.h"
#include "objects/function/BinaryOperator.h"

enum Presidences {
	ELEMENT_BINDING,
	ADDITION,
	SUBTRACTION = ADDITION,
	MULTIPLICATION,
	DIVISION = MULTIPLICATION,
	POWER,
	SET_BINDING
};

namespace SlateDefinitions {
	
	extern std::vector<std::string> symbolBases;
	extern std::vector<std::string> symbolFlares;
	extern std::vector<std::string> controlSeqeuenceCharacters;
	extern std::vector<std::string> binaryOperators;
	extern std::unordered_map<std::string, std::string> controlSequenceFunctions;

	extern std::unordered_map<std::string, Object*> defaultDefinitions;

	void load();

}