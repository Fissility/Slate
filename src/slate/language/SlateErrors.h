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

class CompileEmptyFlare : public SlateError {
public:

	CompileEmptyFlare(size_t b, size_t e) :SlateError(b, e) {
		info = std::format("Empty flare at {}-{}", locationBegin, locationEnd);
	}

};

class CompileUnclosedGroup : public SlateError {
public:

	CompileUnclosedGroup(size_t b, size_t e) :SlateError(b, e) {
		info = std::format("Group starting at {}-{} is not closed.", locationBegin, locationEnd);
	}

};

class CompileUnopenedGroup : public SlateError {
public:

	CompileUnopenedGroup(size_t b, size_t e) :SlateError(b, e) {
		info = std::format("Group ending at {}-{} was never opened", locationBegin, locationEnd);
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

class CompileControlSequenceFunctionNotEnoughArguments : public SlateError {
public:

	CompileControlSequenceFunctionNotEnoughArguments(size_t b, size_t e) : SlateError(b, e) {
		info = std::format("Function at {}-{} has not enough arguments.", locationBegin, locationEnd);
	}

};

class CompileControlSequenceFunctionUnclosedBracket : public SlateError {
public:

	CompileControlSequenceFunctionUnclosedBracket(size_t b, size_t e) : SlateError(b, e) {
		info = std::format("Function at {}-{} is missing control sequence argument ending.", locationBegin, locationEnd);
	}

};

class CompileFunctionMissingArguments : public SlateError {
public:

	CompileFunctionMissingArguments(size_t b, size_t e) : SlateError(b, e) {
		info = std::format("Function at {}-{} has no arguments.", locationBegin, locationEnd);
	}

};

class CompileDidNotUnderstandExpression : public SlateError {
public:
	CompileDidNotUnderstandExpression() : SlateError(0,0) {
		info = "The expression could not be understood. If you see this error, then something went terribly wrong! Please report this!";
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