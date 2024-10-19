#pragma once

// Lexer
/*
#define OUT_OF_PLACE(S,E) {-1, std::format("Out of place symbol at {}",S),S,E}
#define EMPTY_SUBSCRIPT(S,E) { -2, std::format("Empty subscript at {}",S),S,E}
#define UNCLOSED_SUBSCRIPT(S,E) { -3, std::format("Unclosed subscript at {}",S),S,E}
#define EMPTY_FLARE(S,E) {-4, std::format("Empty flare at {}",S),S,E}
#define UNCLOSED_FLARE(S,E) {-5, std::format("Unclosed flare at {}",S),S,E}
#define UNCLOSED_FRACTION(S,E) {-6, std::format("Unclosed fraction at {}",S),S,E}

#define BRACKET_NOT_CLOSED(S,E) {-6, std::format("Bracket at {} was not closed",i),S),S,E}
#define BRACKET_NOT_OPENED(S,E) {-7, std::format("Bracket at {} was nenver opened",S),S,E}
#define OPERATOR_NOT_DEFINED(S,E) {-8, std::format("Operator at {} was never defined",E),S,E}
#define FLOATING_OPERATOR(NAME,S,E) {-9, std::format("Operator {} doesn't have elements on both sides",NAME),S,E}
#define DOMAIN_EXCEPTION(NAME,S,E) {-9 , std::format("The domain on which {} is defined does not support the attempted operation",NAME),S,E}*/

class SlateError : public std::exception {
public:
	size_t locationBegin;
	size_t locationEnd;
	SlateError(size_t b, size_t e) {
		locationBegin = b;
		locationEnd = e;
	}
};

class CompileOutOfPlace : public SlateError {
public:

	CompileOutOfPlace(size_t b, size_t e):SlateError(b,e) {}

	const char* what() const override {
		return "TODO:write decp";
	}
};

class CompileEmptySubscript : public SlateError {
public:

	CompileEmptySubscript(size_t b, size_t e) :SlateError(b, e) {}

	const char* what() const override {
		return "TODO:write decp";
	}
};

class CompileUnclosedSubscript : public SlateError {
public:

	CompileUnclosedSubscript(size_t b, size_t e) :SlateError(b, e) {}

	const char* what() const override {
		return "TODO:write decp";
	}
};

class CompileEmptyFlare : public SlateError {
public:

	CompileEmptyFlare(size_t b, size_t e) :SlateError(b, e) {}

	const char* what() const override {
		return "TODO:write decp";
	}
};

class CompileUnclosedFlare : public SlateError {
public:

	CompileUnclosedFlare(size_t b, size_t e) :SlateError(b, e) {}

	const char* what() const override {
		return "TODO:write decp";
	}
};

class CompileUnclosedFraction : public SlateError {
public:

	CompileUnclosedFraction(size_t b, size_t e) :SlateError(b, e) {}

	const char* what() const override {
		return "TODO:write decp";
	}
};

class CompileBracketNotClosed : public SlateError {
public:

	CompileBracketNotClosed(size_t b, size_t e) :SlateError(b, e) {}

	const char* what() const override {
		return "TODO:write decp";
	}
};

class CompileBracketNotOpened : public SlateError {
public:

	CompileBracketNotOpened(size_t b, size_t e) :SlateError(b, e) {}

	const char* what() const override {
		return "TODO:write decp";
	}
};

class CompileOperatorNotDefinied : public SlateError {
public:

	CompileOperatorNotDefinied(size_t b, size_t e) : SlateError(b, e) {}

	const char* what() const override {
		return "TODO:write decp";
	}
};

class CompileFloatingOperator : public SlateError {
public:

	CompileFloatingOperator(size_t b, size_t e) : SlateError(b, e) {}

	const char* what() const override {
		return "TODO:write decp";
	}
};

class CompileDomainException : public SlateError {
public:

	CompileDomainException(size_t b, size_t e) : SlateError(b, e) {}

	const char* what() const override {
		return "TODO:write decp";
	}
};

class RuntimeDomainException : public SlateError {
public:

	RuntimeDomainException(size_t b, size_t e) : SlateError(b, e) {}

	const char* what() const override {
		return "TODO:write decp";
	}
};

class RuntimeNoInverse : public SlateError {
public:

	RuntimeNoInverse(size_t b, size_t e) : SlateError(b, e) {}

	const char* what() const override {
		return "TODO:write decp";
	}
};