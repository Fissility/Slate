#pragma once

#include "../Object.h"
#include "../set/Set.h"

#include <functional>

enum InputTypes {
	NORMAL,
	BINARY_OPERATOR
};

typedef size_t InputType;

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