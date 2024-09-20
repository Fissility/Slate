#pragma once

#include "Function.h"

class IdentityFunction : public Function {
public:


	IdentityFunction(Set* domain, Set* codomain) : Function(domain, codomain,
		[](Object* o) {
			return o;
		}
	)
	{}

};