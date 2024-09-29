#pragma once

struct ParseError {
public:
	int id;
	std::string name;
	size_t begin;
	size_t end;
	ParseError(int id, std::string name, size_t begin, size_t end) {
		this->id = id;
		this->name = name;
		this->begin = begin;
		this->end = end;
	}
};

typedef size_t TokenType;
typedef size_t Operator;

enum TokenTypes {
	SYMBOL,					// Variable/function name
	OPERATOR,				// Operator name, mostly included here for better syntax highlighting
	BEGIN_SCOPE,			// (
	END_SCOPE,				// )
	EQUALS,					// =
							// VVVVVV
	FRACTION_BEGIN_FIRST,	// \frac{ ....
							//			   VV
	FRACTION_BEGIN_SECOND,	// \frac{ .... }{ ....
							//                     V
	FRACTION_END,			// \frac{ .... }{ .... }

	END_COUNT
};

struct Token {
public:
	TokenType type;
	size_t begin;
	size_t end;
	Token(TokenType type, size_t begin, size_t end) {
		this->type = type;
		this->begin = begin;
		this->end = end;
	}
};

enum LexerFlags {
	FRACTION_OPEN_TOP,
	FRACTION_OPEN_BOTTOM
};

enum SyntaxWrapperTypes {
	INDEPENDENT,
	DEPENDENT,
	UNKNOWN,
	MARKER
};

enum MarkerTypes {
	BEGIN_SCOPE,
	END_SCOPE,
	EQUALS
};

typedef size_t SyntaxWrapperType;
typedef size_t MarkerType;

class ObjectSyntaxWrapper {

public:

	SyntaxWrapperType type;

	ObjectSyntaxWrapper(SyntaxWrapperType type) {
		this->type = type;
	}

};

class Independent : public ObjectSyntaxWrapper {

public:

	Object* o;

	Independent(Object* o) : ObjectSyntaxWrapper(INDEPENDENT) {
		this->o = o;
	}

};

class Unknown : public ObjectSyntaxWrapper {

public:

	std::string name;

	Unknown(std::string name) : ObjectSyntaxWrapper(UNKNOWN) {
		this->name = name;
	}

};

class Dependent : public ObjectSyntaxWrapper {
public:

	Function* o;
	std::vector<Unknown> depedencies;
	Dependent(Function* o) : ObjectSyntaxWrapper(DEPENDENT) {
		this->o = o;
	}
	void addDependecy(Unknown u) {
		depedencies.push_back(u);
	}

};

class Marker : public ObjectSyntaxWrapper {
public:

	MarkerType mType;

	Marker(MarkerType mType) : ObjectSyntaxWrapper(MARKER) {
		this->mType = mType;
	}

};