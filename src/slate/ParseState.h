#pragma once

#include "objects/Expression.h"

typedef size_t TokenType;
typedef size_t Operator;

namespace TokenTypes {
	enum TokenTypes {
		NUMERICAL_CONSTANT,		// Constant real numbers
		SYMBOL,					// Variable/function name
		BEGIN_SCOPE,			// (
		END_SCOPE,				// )

		END_COUNT
	};
}

struct StringLocation {
	size_t begin;
	size_t end;

	bool operator==(const StringLocation& other) const {
		return other.begin == begin && other.end == end;
	}
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
		COMMA
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
	bool canBeFunctionOrExpression = false;

	Unknown(std::string name, StringLocation location, size_t nestingLevel) : ObjectSyntaxWrapper(SyntaxWrapperTypes::UNKNOWN, nestingLevel) {
		this->name = name;
		this->location = location;
	}

};

// Not quite operators
class Marker : public ObjectSyntaxWrapper {
public:

	MarkerType mType;

	Marker(MarkerType mType, StringLocation location, size_t nestingLevel) : ObjectSyntaxWrapper(SyntaxWrapperTypes::MARKER,  nestingLevel) {
		this->mType = mType;
		this->location = location;
	}

};


// ===========

namespace NodeTypes {
	enum NodeTypes {
		F, // FUNCTION
		J, // JOIN
		Q, // REVERSE JOIN
		C, // CONSTANT
		U  // UNKNOWN
	};
}

typedef size_t NodeType;

class Node {
public:
	NodeType type;
	std::vector<Node*> head;
	std::vector<Node*> tail;
};

class FNode : public Node {
public:
	Function* function;
	FNode(Function* function) {
		this->type = NodeTypes::F;
		this->function = function;
	}
};

class JNode : public Node {
public:
	size_t nestingLevel;
	JNode(size_t nestingLevel) {
		this->type = NodeTypes::J;
		this->nestingLevel = nestingLevel;
	}
};

class QNode : public Node {
public:
	QNode() {
		this->type = NodeTypes::Q;
	}
};

class CNode : public Node {
public:
	Object* constant;
	CNode(Object* constant) {
		this->type = NodeTypes::C;
		this->constant = constant;
	}
};

class UNode : public Node {
public:
	std::string name;
	UNode(std::string name) {
		this->type = NodeTypes::U;
		this->name = name;
	}
};