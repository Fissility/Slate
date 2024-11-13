#pragma once
#include "Function.h"

class BinaryOperator : public Function {
public:

	size_t precedence;
	bool canBeUnary = false;

	BinaryOperator(Set* domain, Set* codomain, std::function<Object* (Object*)> mapping, int precedence) :
	Function(domain,codomain,mapping)
	{
		this->type = Types::BINARY_OPERATOR;
		this->precedence = precedence;
	}

	void hasUnary(bool unary) {
		this->canBeUnary = unary;
	}



};