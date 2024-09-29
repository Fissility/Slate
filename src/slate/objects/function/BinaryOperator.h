#pragma once
#include "Function.h"

class BinaryOperator : public Function {
public:

	size_t precedence;

	BinaryOperator(Set* domain, Set* codomain, std::function<Object* (Object*)> mapping, int precedence) :
	Function(domain,codomain,mapping)
	{
		this->type = Types::BINARY_OPERATOR;
		this->precedence = precedence;
	}

	BinaryOperator(Set* domain, Set* codomain, int precedence) :
	Function(domain, codomain) 
	{
		this->precedence = precedence;
	}

};