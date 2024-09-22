#pragma once

struct ParseError {
public:
	int id;
	std::string name;
	ParseError(int id, std::string name) {
		this->id = id;
		this->name = name;
	}
};

enum TokenTypes {
	SYMBOL,					// Variable name
	BEGIN_SCOPE,			// (
	END_SCOPE,				// )
	EQUALS,					// =
	ADDITION,				// +
	SUBTRACTION,			// -
	MULTIPILCATION,			// \cdot
	DIVSION,				// \div
	SUPERSCRIPT,			// ^
							// VVVVVV
	FRACTION_BEGIN_FIRST,	// \frac{ ....
							//			   VV
	FRACTION_BEGIN_SECOND,	// \frac{ .... }{ ....
							//                     V
	FRACTION_END,			// \frac{ .... }{ .... }
	COLON,					// :
	VERTICAL_SEPARATOR,		// |
	MAPS_TO,				// \Rightarrow
	CROSS,					// \times
	FOR_ALL,				// \forall
	IN,						// \in
	COMMA,					// ,
};

typedef size_t TokenType;

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

