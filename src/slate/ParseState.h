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
	Token(TokenType type, size_t begin, size_t end) {
		this->type = type;
		this->begin = begin;
		this->end = end;
	}
};