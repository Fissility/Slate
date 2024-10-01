#pragma once

#include "../Object.h"
#include "../set/Set.h"

#include <functional>

typedef size_t InputType;

// The function object should not domain check what is passed when evaluating, the domain checking should be done by the evaluator
// Failiure to provide the expected input type will result in undefined behaviour, most probably an exception

class Function : public Object {
public:

	Set* domain;
	Set* codomain;

	std::function<Object* (Object*)> mapping;

	bool implemented = false;


	Function(Set* domain, Set* codomain, std::function<Object* (Object*)> mapping) {
		this->type = Types::FUNCTION;
		this->mapping = mapping;
		this->domain = domain;
		this->codomain = codomain;
		this->implemented = true;
	}

	Function(Set* domain, Set* codomain) {
		this->type = Types::FUNCTION;
		this->domain = domain;
		this->codomain = codomain;
		this->implemented = false;
	}

	Object* evaluate(Object* o) {
		return mapping(o);
	}

};