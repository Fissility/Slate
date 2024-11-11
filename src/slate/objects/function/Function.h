#pragma once

#include "../Object.h"
#include "../set/Set.h"
#include "Form.h"

#include <functional>

typedef size_t InputType;

// The function object should not domain check what is passed when evaluating, the domain checking should be done by the evaluator
// Failiure to provide the expected input type will result in undefined behaviour, most probably an exception

class Function : public Object {
public:

	Set* domain;
	Set* codomain;

	std::function<Object* (Object*)> mapping;
	std::function<Function* (InputForm,Object*[])> inverseBuilder;


	bool implemented = false;
	bool hasInverse = false;


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

	void addInverse(std::function<Function* (InputForm,Object*[])> iB) {
		inverseBuilder = iB;
		hasInverse = true;
	}

	Function* inverse(InputForm form,Object* constants[]) {
		if (!hasInverse) return nullptr;
		return inverseBuilder(form,constants);
	}

	std::string toString() {
		return "<FUNCTION>";
	}

};