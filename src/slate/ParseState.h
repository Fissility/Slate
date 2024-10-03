#pragma once

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

	ObjectSyntaxWrapper(SyntaxWrapperType type, Token* assosciatedToken) {
		this->type = type;
		this->assosciatedToken = assosciatedToken;
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

	std::vector<StringLocation> locations;

	Known(Object* o, KnownKind kind) : ObjectSyntaxWrapper(SyntaxWrapperTypes::KNOWN, assosciatedToken) {
		this->o = o;
		this->kind = kind;
	}

	void addDebugLocation(StringLocation location) {
		locations.push_back(location);
	}
	void addDebugLocationsFrom(Known* from) {
		for (StringLocation& l : from->locations) {
			addDebugLocation(l);
		}
	}

};

class Unknown : public ObjectSyntaxWrapper {

public:

	std::string name;
	StringLocation location;

	Unknown(std::string name, StringLocation location) : ObjectSyntaxWrapper(SyntaxWrapperTypes::UNKNOWN, assosciatedToken) {
		this->name = name;
		this->location = location;
	}

};

class Dependent : public ObjectSyntaxWrapper {
public:

	Function* o;
	std::vector<Unknown*> depedencies;
	std::vector<StringLocation> locations;
	StringLocation op;
	Dependent(Function* o, StringLocation op) : ObjectSyntaxWrapper(SyntaxWrapperTypes::DEPENDENT, assosciatedToken) {
		this->o = o;
		this->op = op;
	}
	void addDependecy(Unknown* u) {
		depedencies.push_back(u);
	}
	void addAllDependeciesFrom(Dependent& d) {
		for (Unknown* u : d.depedencies) {
			addDependecy(u);
		}
	}
	size_t uknownsCount() {
		return depedencies.size();
	}
	void addDebugLocation(StringLocation location) {
		this->locations.push_back(location);
	}
	void addDebugLocationsFrom(Dependent* from) {
		addDebugLocation(from->op);
		for (StringLocation& l : from->locations) {
			addDebugLocation(l);
		}
	}
	void addDebugLocationsFrom(Known* from) {
		for (StringLocation& l : from->locations) {
			addDebugLocation(l);
		}
	}

};

class Marker : public ObjectSyntaxWrapper {
public:

	MarkerType mType;

	Marker(MarkerType mType, Token* assosciatedToken) : ObjectSyntaxWrapper(SyntaxWrapperTypes::MARKER, assosciatedToken) {
		this->mType = mType;
	}

};