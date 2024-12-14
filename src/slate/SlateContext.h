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

	void printNode(SlateLanguage::AST::Node* n, size_t spaces = 0);

	bool equalsNodes(SlateLanguage::AST::Node* first, SlateLanguage::AST::Node* second);
	bool equalsAST(SlateLanguage::AST::Node* first, SlateLanguage::AST::Node* second);
	bool superimposePattern(SlateLanguage::AST::Node* head, SlateLanguage::AST::Node* inputPattern, std::unordered_map<std::string,SlateLanguage::AST::Node*>& patternTemplateInputs);
	void populatePattern(SlateLanguage::AST::Node* replacement, std::unordered_map<std::string, SlateLanguage::AST::Node*>& patternTemplateInputs);
	SlateLanguage::AST::Node* simplifyTree(SlateLanguage::AST::Node* head);

};