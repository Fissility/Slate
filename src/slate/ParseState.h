#pragma once

#include "objects/Expression.h"

typedef size_t TokenType;
typedef size_t Operator;

namespace TokenTypes {
	enum TokenTypes {
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
		COLON
	};
}

typedef size_t SyntaxWrapperType;
typedef size_t MarkerType;

class ObjectSyntaxWrapper {

public:

	SyntaxWrapperType type;
	// This is needed for error throwing so in the later stages errors can still point to a location in the initial string
	Token* assosciatedToken;
	size_t nestingLevel = 0;

	ObjectSyntaxWrapper(SyntaxWrapperType type, Token* assosciatedToken, size_t nestingLevel) {
		this->type = type;
		this->assosciatedToken = assosciatedToken;
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

	StringLocation location;

	Known(Object* o, KnownKind kind, StringLocation location, size_t nestingLevel) : ObjectSyntaxWrapper(SyntaxWrapperTypes::KNOWN, assosciatedToken, nestingLevel) {
		this->o = o;
		this->kind = kind;
		this->location = location;
	}

};

class Unknown : public ObjectSyntaxWrapper {

public:

	std::string name;
	StringLocation location;

	Unknown(std::string name, StringLocation location, size_t nestingLevel) : ObjectSyntaxWrapper(SyntaxWrapperTypes::UNKNOWN, assosciatedToken, nestingLevel) {
		this->name = name;
		this->location = location;
	}

};

class Dependent : public ObjectSyntaxWrapper {
public:

	Expression* exp;
	std::vector<Unknown*> depedencies;
	StringLocation location;
	Dependent(Expression* exp, StringLocation location, size_t nestingLevel) : ObjectSyntaxWrapper(SyntaxWrapperTypes::DEPENDENT, assosciatedToken, nestingLevel) {
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

	Marker(MarkerType mType, Token* assosciatedToken, size_t nestingLevel) : ObjectSyntaxWrapper(SyntaxWrapperTypes::MARKER, assosciatedToken, nestingLevel) {
		this->mType = mType;
	}

};