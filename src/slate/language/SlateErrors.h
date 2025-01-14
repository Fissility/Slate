#pragma once

#include <exception>
#include <stdlib.h>
#include <string>

struct StringLocation {
  std::size_t begin;
  std::size_t end;

  bool operator==(const StringLocation &other) const {
    return other.begin == begin && other.end == end;
  }
};

class SlateError : public std::exception {
public:
  std::size_t locationBegin;
  size_t locationEnd;
  std::string info;
  SlateError(size_t b, size_t e) {
    locationBegin = b;
    locationEnd = e;
  }

  const char *what() const noexcept override { return info.c_str(); }
};

class CompileOutOfPlace : public SlateError {
public:
  CompileOutOfPlace(size_t b, size_t e) : SlateError(b, e) {
    info = "Symbol out of place at " + std::to_string(locationBegin) + "-" +
           std::to_string(locationEnd);
  }
};

class CompileEmptySubscript : public SlateError {
public:
  CompileEmptySubscript(size_t b, size_t e) : SlateError(b, e) {
    info = "Empty symbol subscript at " + std::to_string(locationBegin) + "-" +
           std::to_string(locationEnd);
  }
};

class CompileEmptyFlare : public SlateError {
public:
  CompileEmptyFlare(size_t b, size_t e) : SlateError(b, e) {
    info = "Empty flare at " + std::to_string(locationBegin) + "-" +
           std::to_string(locationEnd);
  }
};

class CompileUnclosedGroup : public SlateError {
public:
  CompileUnclosedGroup(size_t b, size_t e) : SlateError(b, e) {
    info = "Group starting at " + std::to_string(b) + "-" + std::to_string(e) + " is not closed.";
  }
};

class CompileUnopenedGroup : public SlateError {
public:
  CompileUnopenedGroup(size_t b, size_t e) : SlateError(b, e) {
    info = "Group ending at " + std::to_string(b) + "-" + std::to_string(e) + " was never opened";
  }
};

class CompileOperatorNotDefinied : public SlateError {
public:
  CompileOperatorNotDefinied(size_t b, size_t e) : SlateError(b, e) {
    info = "Operator at {} " + std::to_string(b) + "-" + std::to_string(e) + " is not defined";
  }
};

class CompileFloatingOperator : public SlateError {
public:
  CompileFloatingOperator(size_t b, size_t e) : SlateError(b, e) {
    info = "Operator at {} " + std::to_string(b) + "-" + std::to_string(e) + " is floating.";
  }
};

class CompileDomainException : public SlateError {
public:
  CompileDomainException(size_t b, size_t e) : SlateError(b, e) {
    info = "Operation at " + std::to_string(b) + "" + std::to_string(e) + " cannot be performed as the inputs are outside of the defined function domain.";
  }
};

class CompileControlSequenceFunctionNotEnoughArguments : public SlateError {
public:
  CompileControlSequenceFunctionNotEnoughArguments(size_t b, size_t e)
      : SlateError(b, e) {
    info = "Function at " + std::to_string(b) + "" + std::to_string(e) + " has not enough arguments.";
  }
};

class CompileControlSequenceFunctionUnclosedBracket : public SlateError {
public:
  CompileControlSequenceFunctionUnclosedBracket(size_t b, size_t e)
      : SlateError(b, e) {
    info = "Function at " + std::to_string(b) + "" + std::to_string(e) + " is missing control sequence argument ending.";
  }
};

class CompileFunctionMissingArguments : public SlateError {
public:
  CompileFunctionMissingArguments(size_t b, size_t e) : SlateError(b, e) {
    info = "Function at " + std::to_string(b) + "" + std::to_string(e) + " has no arguments.";
  }
};

class CompileDidNotUnderstandExpression : public SlateError {
public:
  CompileDidNotUnderstandExpression() : SlateError(0, 0) {
    info = "The expression could not be understood. If you see this error then something went terribly wrong! Please report the problem!";
  }
};

class RuntimeDomainException : public SlateError {
public:
  RuntimeDomainException(size_t b, size_t e) : SlateError(b, e) {
    info = "Operation at " + std::to_string(b) + "-" + std::to_string(e) + " cannot be performed as the object(s) are outside of the defined function domain. (RUNTIME TRIGGERED)";
  }
};

class RuntimeNoInverse : public SlateError {
public:
  RuntimeNoInverse(size_t b, size_t e) : SlateError(b, e) {
    info = "Function/Expression at " + std::to_string(b) + "-" + std::to_string(e) + " was unable to/can't be inversed. (RUNTIME TRIGGERED)";
  }
};
