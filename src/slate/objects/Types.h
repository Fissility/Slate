#pragma once

#include <string>

typedef size_t Type;

namespace Types {
	enum Types {
		UNKNOWN,
		NUMBER,
		INTERVAL,
		SET,
		FUNCTION,
		BINARY_OPERATOR,
		TUPLE,
		CATEGORY,
		EXPRESSION
	};

	extern std::string getTypeName(Type t);

};