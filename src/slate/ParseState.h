#pragma once

enum ParseStates {
	BEGIN,
	FOUND_SYMBOL_GET_SYMBOL,
	FOUND_FLARE_GET_SYMBOL,
	FLOATING_SYMBOL,
	SYMBOL_ASSIGMENT,
	END
};

typedef size_t ParseState;

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
	SYMBOL,
	EXPRESSION,
	IDENTITY
};

typedef size_t TokenType;

struct Token {
public:
	TokenType type;
	size_t begin;
	size_t end;
	std::string formalForm;
	Token(TokenType type, size_t begin, size_t end) {
		this->type = type;
		this->begin = begin;
		this->end = end;
	}
	Token(TokenType type, size_t begin, size_t end,std::string formalForm) {
		this->type = type;
		this->begin = begin;
		this->end = end;
		this->formalForm = formalForm;
	}
};