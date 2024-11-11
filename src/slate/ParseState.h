#pragma once

#include "objects/Expression.h"

typedef size_t TokenType;
typedef size_t Operator;

namespace TokenTypes {
	enum TokenTypes {
		NUMERICAL_CONSTANT,		// Constant real numbers
		SYMBOL,					// Variable/function name
		OPERATOR,				// Operator name, mostly included here for better syntax highlighting
		BEGIN_SCOPE,			// (
		END_SCOPE,				// )
		// VVVVVV
		FRACTION_BEGIN_FIRST,	// \frac{ ....
		//			   VV
		FRACTION_BEGIN_SECOND,	// \frac{ .... }{ ....
		//                     V
		FRACTION_END,			// \frac{ .... }{ .... }

		END_COUNT
	};
}

struct StringLocation {
	size_t begin;
	size_t end;
};

struct Token {
public:
	TokenType type;
	StringLocation location;
	Token(TokenType type, size_t begin, size_t end) {
		this->type = type;
		location = { begin,end };
	}
};

enum LexerFlags {
	FRACTION_OPEN_TOP,
	FRACTION_OPEN_BOTTOM
};

namespace SyntaxWrapperTypes {
	enum SyntaxWrapperTypes {
		KNOWN,
		DEPENDENT,
		UNKNOWN,
		MARKER
	};
}

namespace MarkerTypes {
	enum MarkerTypes {
		BEGIN_SCOPE,
		END_SCOPE,
		EQUALS,
		COLON,
		RIGHT_ARROW
	};
}

typedef size_t SyntaxWrapperType;
typedef size_t MarkerType;

class ObjectSyntaxWrapper {

public:

	SyntaxWrapperType type;
	// This is needed for error throwing so in the later stages errors can still point to a location in the initial string
	size_t nestingLevel = 0;

	StringLocation location;

	ObjectSyntaxWrapper(SyntaxWrapperType type, size_t nestingLevel) {
		this->type = type;
		this->nestingLevel = nestingLevel;
	}

};

namespace KnownKinds {
	enum KnownKinds {
		OPERATOR,
		BINARY_OPERATOR,
		OPERAND
	};
}

typedef size_t KnownKind;

class Known : public ObjectSyntaxWrapper {
public:

	Object* o;

	KnownKind kind;

	Known(Object* o, KnownKind kind, StringLocation location, size_t nestingLevel) : ObjectSyntaxWrapper(SyntaxWrapperTypes::KNOWN, nestingLevel) {
		this->o = o;
		this->kind = kind;
		this->location = location;
	}

};

class Unknown : public ObjectSyntaxWrapper {

public:

	std::string name;

	Unknown(std::string name, StringLocation location, size_t nestingLevel) : ObjectSyntaxWrapper(SyntaxWrapperTypes::UNKNOWN, nestingLevel) {
		this->name = name;
		this->location = location;
	}

};

class Dependent : public ObjectSyntaxWrapper {
public:

	Expression* exp;
	std::vector<Unknown*> depedencies;
	Dependent(Expression* exp, StringLocation location, size_t nestingLevel) : ObjectSyntaxWrapper(SyntaxWrapperTypes::DEPENDENT, nestingLevel) {
		this->exp = exp;
		this->location = location;
	}
	void addDependecy(Unknown* u) {
		depedencies.push_back(u);
	}
	void addAllDependeciesFrom(Dependent* d) {
		for (Unknown* u : d->depedencies) {
			addDependecy(u);
		}
	}
	size_t uknownsCount() {
		return depedencies.size();
	}

};

// Not quite operators
class Marker : public ObjectSyntaxWrapper {
public:

	MarkerType mType;
	StringLocation location;

	Marker(MarkerType mType, StringLocation location, size_t nestingLevel) : ObjectSyntaxWrapper(SyntaxWrapperTypes::MARKER,  nestingLevel) {
		this->mType = mType;
		this->location = location;
	}

};