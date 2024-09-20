#pragma once

#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include "objects/Object.h"
#include "SlateDefinitions.h"
#include "ParseState.h"

struct ExpressionInfo {
	std::string& line;
	size_t index;
	ExpressionInfo(std::string& line, size_t index) : line(line) {
		this->index = index;
	}
};

class SlateContext {

private:

	std::vector<std::string*> expresions;

public:

	std::unordered_map<std::string, Object*> nameMap;

	ExpressionInfo newExpression();
	void parse();
	ParseError parseLine(std::string& line);
	void removeExpresion(size_t index);
	void brk();

};