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

	InputType inputType;

	Set* domain;
	Set* codomain;

	std::function<Object* (Object*)> mapping;


	Function(Set* domain, Set* codomain, std::function<Object* (Object*)> mapping, InputType iType = InputTypes::NORMAL) {
		this->mapping = mapping;
		this->inputType = iType;
		this->domain = domain;
		this->codomain = codomain;
	}


	Object* evaluate(Object* o) {
		return mapping(o);
	}

};