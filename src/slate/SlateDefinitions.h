#pragma once

#include <vector>
#include <string>
#include "objects/function/Function.h"
#include "objects/set/NSet.h"
#include "objects/set/ZSet.h"
#include "objects/set/QSet.h"
#include "objects/set/RSet.h"


namespace SlateDefinitions {

	extern std::vector<std::string> symbolBases;
	extern std::vector<std::string> symbolFlares;
	extern std::vector<std::string> specialCharacters;

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

	void load();

}