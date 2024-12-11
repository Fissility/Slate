#pragma once

#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include "objects/Expression.h"
#include "language/Parser.h"
#include "objects/Object.h"
#include "SlateDefinitions.h"

class SlateContext {
private:

	std::string displayStringFromAST(SlateLanguage::AST::Node* head);
	Expression* expressionFromAST(SlateLanguage::AST::Node* head, std::vector<std::string>* unknowns);

public:

	Definitions definitions;

	std::string getObjectName(Object* o);

	Object* processSyntaxLine(std::string line);

};