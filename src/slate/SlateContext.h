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

  std::unordered_map<SlateLanguage::AST::Node*,std::vector<size_t>> simplificationSignatures;

  bool doSignaturesMatch(std::vector<size_t>& expression, std::vector<size_t>& simplification); // TODO: Implement it
  bool maybeDoSimplification(SlateLanguage::AST::Node* head, Simplification& s); // TODO: Implement it

public:

	Definitions definitions;

	std::string getObjectName(Object* o);

  void addDefinitions(Definitions& defs);
	Object* interpret(std::string line);

	void printNode(SlateLanguage::AST::Node* n, size_t spaces = 0);

	bool equalsNodes(SlateLanguage::AST::Node* first, SlateLanguage::AST::Node* second);
	bool equalsAST(SlateLanguage::AST::Node* first, SlateLanguage::AST::Node* second);
  void swapHeads(SlateLanguage::AST::Node* target, SlateLanguage::AST::Node* source);
	bool checkPatternHead(SlateLanguage::AST::Node* head, SlateLanguage::AST::Node* inputPattern, std::unordered_map<std::string,SlateLanguage::AST::Node*>& patternTemplateInputs);
  void populatePattern(SlateLanguage::AST::Node*& replacement, std::unordered_map<std::string, SlateLanguage::AST::Node*>& patternTemplateInputs);
  bool simplifyTree(SlateLanguage::AST::Node*& head);
  bool reduceTree(SlateLanguage::AST::Node*& top, SlateLanguage::AST::Node*& head, std::vector<SlateLanguage::AST::Node*>& previous);
  void getRepetitionSignature(SlateLanguage::AST::Node* head, std::vector<SlateLanguage::AST::Node*>& subSection, std::vector<size_t>& sameCount); 

};
