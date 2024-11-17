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


	SlateContext();
	SlateContext(Definitions defaultDefinitions) {
		this->defaultDefinitions = defaultDefinitions;
	}

	Definitions defaultDefinitions;
	Definitions definitions;

	bool nameExists(std::string& name);
	Object* getObject(std::string& name);
	std::string getObjectName(Object* o);

	ExpressionInfo newExpression();
	std::vector<Object*> processSyntax();
	Object* processSyntaxLine(std::string& line);
	void lexer(std::string& line, std::vector<Token>& tokens);
	void linkTokensToObjects(std::string line, std::vector<Token>& tokens, std::vector<ObjectSyntaxWrapper*>& objects);
	void parser(std::vector<ObjectSyntaxWrapper*>& wrappers);
	void removeExpresion(size_t index);

	void brk();

};