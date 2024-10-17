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
	SET_BINDING
};

namespace SlateDefinitions {
	
	extern std::vector<std::string> symbolBases;
	extern std::vector<std::string> symbolFlares;
	extern std::vector<std::string> specialCharacters;
	extern std::vector<std::string> binaryOperators;

	extern AllSetsSet* AllSets_set;
	extern Set* AllSets2_set;
	extern USet* U_set;
	extern Set* U2_set;
	extern NSet* N_set;
	extern ZSet* Z_set;
	extern QSet* Q_set;
	extern RSet* R_set;
	extern Set* R2_set;
	extern Function* addition_func;
	extern Function* subtraction_func;
	extern Function* multiplication_func;
	extern Function* division_func;
	extern Function* power_func;
	extern Function* setCategoryBinding_func;

	void load();

}