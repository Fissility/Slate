#pragma once

#include <format>

class SlateError : public std::exception {
public:
	size_t locationBegin;
	size_t locationEnd;
	std::string info;
	SlateError(size_t b, size_t e) {
		locationBegin = b;
		locationEnd = e;
	}

	const char* what() const override {
		return info.c_str();
	}
};

class CompileOutOfPlace : public SlateError {
public:

	CompileOutOfPlace(size_t b, size_t e):SlateError(b,e) {
		info = std::format("Symbol out of place at {}-{}", locationBegin, locationEnd);
	}
};

class CompileEmptySubscript : public SlateError {
public:

	CompileEmptySubscript(size_t b, size_t e) :SlateError(b, e) {
		info = std::format("Empty symbol subscript at {}-{}", locationBegin, locationEnd);
	}

};

class CompileUnclosedSubscript : public SlateError {
public:

	CompileUnclosedSubscript(size_t b, size_t e) :SlateError(b, e) {
		info = std::format("Unclosed symbol subscript at {}-{}", locationBegin, locationEnd);
	}

};

class CompileEmptyFlare : public SlateError {
public:

	CompileEmptyFlare(size_t b, size_t e) :SlateError(b, e) {
		info = std::format("Empty flare at {}-{}", locationBegin, locationEnd);
	}

};

class CompileUnclosedFlare : public SlateError {
public:

	CompileUnclosedFlare(size_t b, size_t e) :SlateError(b, e) {
		info = std::format("Unclosed flare at {}-{}", locationBegin, locationEnd);
	}

};

class CompileUnclosedFraction : public SlateError {
public:

	CompileUnclosedFraction(size_t b, size_t e) :SlateError(b, e) {
		info = std::format("Unclosed \\frac at {}-{}", locationBegin, locationEnd);
	}

};

class CompileBracketNotClosed : public SlateError {
public:

	CompileBracketNotClosed(size_t b, size_t e) :SlateError(b, e) {
		info = std::format("Bracket not closed for {}-{}", locationBegin, locationEnd);
	}

};

class CompileBracketNotOpened : public SlateError {
public:

	CompileBracketNotOpened(size_t b, size_t e) :SlateError(b, e) {
		info = std::format("Bracket not opened for {}-{}", locationBegin, locationEnd);
	}

};

class CompileOperatorNotDefinied : public SlateError {
public:

	CompileOperatorNotDefinied(size_t b, size_t e) : SlateError(b, e) {
		info = std::format("Operator at {}-{} is not defined", locationBegin, locationEnd);
	}

};

class CompileFloatingOperator : public SlateError {
public:

	CompileFloatingOperator(size_t b, size_t e) : SlateError(b, e) {
		info = std::format("Operator at {}-{} is floating.", locationBegin, locationEnd);
	}

};

class CompileDomainException : public SlateError {
public:

	CompileDomainException(size_t b, size_t e) : SlateError(b, e) {
		info = std::format("Operation at {}-{} cannot be performed as the object(s) are outside of the defined function domain.", locationBegin, locationEnd);
	}

};

class RuntimeDomainException : public SlateError {
public:

	RuntimeDomainException(size_t b, size_t e) : SlateError(b, e) {
		info = std::format("Operation at {}-{} cannot be performed as the object(s) are outside of the defined function domain. (RUNTIME TRIGGERED)", locationBegin, locationEnd);
	}

};

class RuntimeNoInverse : public SlateError {
public:

	RuntimeNoInverse(size_t b, size_t e) : SlateError(b, e) {
		info = std::format("Function/Expression at {}-{} was unable to/can't be inversed. (RUNTIME TRIGGERED)", locationBegin, locationEnd);
	}

};