#pragma once

#include "Object.h"
#include <functional>

/*
* Describes a procedure derived from a string of numbers,variables,operators etc.
* Can be evaluated by calling the expression with the values that should be put in the places of the variables
* It is not a function, does not descrbe a mapping, more like a macro
*/
class Expression : public Object {

public:

	size_t numberOfVariables;
	std::function<Object* (Object**)> evalFunc;

	bool hasInverse = false;
	std::function<void(Object*, Object*[])> reverseFunc;

	Expression(size_t numberOfVariables, std::function<Object* (Object**)> evalFunc) {
		this->type = Types::EXPRESSION;
		this->numberOfVariables = numberOfVariables;
		this->evalFunc = evalFunc;
	}

	void setInverse(std::function<void(Object*, Object*[])> reverseFunc) {
		this->reverseFunc = reverseFunc;
		hasInverse = true;
	}

};