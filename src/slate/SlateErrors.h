#pragma once


struct ParseError {
public:
	int id;
	size_t begin;
	size_t end;
	ParseError(int id, size_t begin, size_t end) {
		this->id = id;
		this->begin = begin;
		this->end = end;
	}
};

// Lexer
/*
#define OUT_OF_PLACE(S,E) {-1, std::format("Out of place symbol at {}",S),S,E}
#define EMPTY_SUBSCRIPT(S,E) { -2, std::format("Empty subscript at {}",S),S,E}
#define UNCLOSED_SUBSCRIPT(S,E) { -3, std::format("Unclosed subscript at {}",S),S,E}
#define EMPTY_FLARE(S,E) {-4, std::format("Empty flare at {}",S),S,E}
#define UNCLOSED_FLARE(S,E) {-5, std::format("Unclosed flare at {}",S),S,E}
#define UNCLOSED_FRACTION(S,E) {-6, std::format("Unclosed fraction at {}",S),S,E}

// Parser

#define BRACKET_NOT_CLOSED(S,E) {-6, std::format("Bracket at {} was not closed",i),S),S,E}
#define BRACKET_NOT_OPENED(S,E) {-7, std::format("Bracket at {} was nenver opened",S),S,E}
#define OPERATOR_NOT_DEFINED(S,E) {-8, std::format("Operator at {} was never defined",E),S,E}
#define FLOATING_OPERATOR(NAME,S,E) {-9, std::format("Operator {} doesn't have elements on both sides",NAME),S,E}
#define DOMAIN_EXCEPTION(NAME,S,E) {-9 , std::format("The domain on which {} is defined does not support the attempted operation",NAME),S,E}*/

// Lexer

namespace ParseErrorCodes {
	enum ParseErrorCodes {
		OK, // This is the good one, the rest are evil

		// Related to the Lexer

		OUT_OF_PLACE,
		EMPTY_SUBSCRIPT,
		UNCLOSED_SUBSCRIPT,
		EMPTY_FLARE,
		UNCLOSED_FLARE,
		UNCOSED_FRACTION,

		// Related to object linking

		OPERATOR_NOT_DEFINED,

		// Related to shuting yard step

		BRACKET_NOT_CLOSED,
		BRACKET_NOT_OPENED,

		// Related to object forming

		FLOATING_OPERATOR,
		NOT_IN_DOMAIN
	};
}

#define OK ParseError(ParseErrorCodes::OK,0,0)
#define OUT_OF_PLACE(S,E) ParseError(ParseErrorCodes::OUT_OF_PLACE,S,E)
#define EMPTY_SUBSCRIPT(S,E) ParseError(ParseErrorCodes::EMPTY_SUBSCRIPT,S,E)
#define UNCLOSED_SUBSCRIPT(S,E) ParseError(ParseErrorCodes::UNCLOSED_SUBSCRIPT,S,E)
#define EMPTY_FLARE(S,E) ParseError(ParseErrorCodes::EMPTY_FLARE,S,E)
#define UNCLOSED_FLARE(S,E) ParseError(ParseErrorCodes::UNCLOSED_FLARE,S,E)
#define UNCLOSED_FRACTION(S,E) ParseError(ParseErrorCodes::UNCOSED_FRACTION,S,E)
#define BRACKET_NOT_CLOSED(S,E) ParseError(ParseErrorCodes::BRACKET_NOT_CLOSED,S,E)
#define BRACKET_NOT_OPENED(S,E) ParseError(ParseErrorCodes::BRACKET_NOT_OPENED,S,E)
#define OPERATOR_NOT_DEFINED(S,E) ParseError(ParseErrorCodes::OPERATOR_NOT_DEFINED,S,E)
#define FLOATING_OPERATOR(S,E) ParseError(ParseErrorCodes::FLOATING_OPERATOR,S,E)
#define DOMAIN_EXCEPTION(S,E) ParseError(ParseErrorCodes::NOT_IN_DOMAIN,S,E)