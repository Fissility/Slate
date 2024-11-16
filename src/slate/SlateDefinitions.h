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

std::string normaliseName(std::string name) {
	std::string normal;
	bool lastWasBackS = false;
	for (size_t i = 0; i < name.size(); i++) {
		char c = name[i];
		switch (c) {
			case '{':
			case '}': {
				if (lastWasBackS) {
					normal += c;
					lastWasBackS = false;
				}
				break;
			}
			case ' ':
				break;
			case '\\': {
				lastWasBackS = true;
				normal += c;
				break;
			}
			default: {
				lastWasBackS = false;
				normal += c;
			}
		}
	}
	return normal;
}

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

namespace SlateDefinitions {
	
	extern std::vector<std::string> symbolBases;
	extern std::vector<std::string> symbolFlares;
	extern std::vector<std::string> controlSeqeuenceCharacters;
	extern std::vector<std::string> binaryOperators;
	extern std::unordered_map<std::string, std::string> controlSequenceFunctions;

	extern std::unordered_map <std::string, Object*> defaultDefinitions;
	extern std::unordered_map <Object*, std::string> defaultNames;

	void load();

}