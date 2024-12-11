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

	// The word definition here refers to the name - object assosciation
	// Map of objects and the names by which they are identified
	std::unordered_map <std::string, Object*> definitions;
	// Map of the printable strings of some objects (i.e. not all objects are required to have a printable string stored)
	std::unordered_map <Object*, std::string> stringValues;

	/*
	* @brief Adds a new definition.
	* @param name = The name of the new object added
	* @param o = The new object
	*/
	void registerDefinition(std::string& name, Object* o);
	/*
	* @param name = The name of the definition
	* @return Returns TRUE if the name is associated with a definition, FALSE otherwise
	*/
	bool definitionExists(std::string& name);
	/*
	* @param name = The name of the definition
	* @return Returns the object assosciated with the definition. Returns nullptr if the definition doesn't exist.
	*/
	Object* getDefinition(std::string& name);

	/*
	* @brief Registers a display string mapping for an object
	*/
	void registerString(Object* o, std::string string);
	/*
	* @param o = The object for which it should check if a name mapping exists
	* @return Returns TRUE if the object has a display name, FALSE otherwise
	*/
	bool objectHasString(Object* o);
	/*
	* @param o = The object for which the display name should be returned
	* @return Returns the display name for the inputted object. Returns "[Object cannot be displayed]" if it does not have a string registered.
	*/
	std::string getString(Object* o);

	/*
	* @brief Registeres an object for which its definition name and display string are the same
	* @param o = The object which should be registered
	* @param name = The name of the object
	*/
	void registerBaseObject(Object* o, std::string name);

	/*
	* @brief Clears all definitions and display strings
	*/
	void clear();

	/*
	* @brief Adds the definitions and display strings from other set of definitions to this one. If there are duplicates of any kind then the entries are not modified.
	*/
	void addFrom(Definitions other) {
		definitions.insert(other.definitions.begin(),other.definitions.end());
		stringValues.insert(other.stringValues.begin(), other.stringValues.end());
	}
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