#include "SlateContext.h"
#include "SlateDefinitions.h"

#include "language/SlateErrors.h"
#include "language/Tokenizer.h"
#include "language/Lexer.h"
#include "language/Parser.h"
#include "objects/Expression.h"
#include "objects/tuple/BiCategory.h" //X
#include <iostream>
#include <stack>
#include "SlateType.h"

using namespace SlateLanguage;

std::string SlateContext::getObjectName(Object* o) {
	if (definitions.objectHasString(o)) {
		return definitions.getString(o);
	}
	switch (o->type) {
		case Types::NUMBER: {
			return std::to_string(((Number*)o)->value);
		}
		case Types::TUPLE: {
			Tuple* t = (Tuple*)o;
			std::string out = "(";
			for (size_t i = 0; i < t->length; i++) {
				out += getObjectName((*t)[i]);
				if (i != t->length - 1) out += ",";
			}
			out += ")";
			return out;
		}
		default: {
			return "<" + Types::getTypeName(o->type) + ">";
		}
	}
}

// TODO: account that some control sequences don't have standard paramters \frac, \sqrt etc
std::string SlateContext::displayStringFromAST(AST::Node* head) {
	if (head->type == AST::NodeTypes::C) {
		Object* constant = ((AST::CNode*)head)->constant;
		return getObjectName(constant);
	}
	if (head->type == AST::NodeTypes::U) {
		return ((AST::UNode*)head)->name;
	}
	std::vector<std::string> tailStrings;
	for (AST::Node* t : head->tail) tailStrings.push_back(displayStringFromAST(t));
	if (head->type == AST::NodeTypes::F) {
		Function* function = ((AST::FNode*)head)->function;
		if (function->type == Types::BINARY_OPERATOR) {
			return tailStrings[0]; // This case is already processed in the required J Node before it
		}
		else {
			return "\\left(" +getObjectName(function) + tailStrings[0] + "\\right)";
		}
	}
	if (head->type == AST::NodeTypes::J) {
		if (!head->head.empty() && head->head[0]->type == AST::NodeTypes::F && ((AST::FNode*)head->head[0])->function->type == Types::BINARY_OPERATOR) {
			return "\\left(" + tailStrings[0] + " " + getObjectName(((AST::FNode*)head->head[0])->function) + " " + tailStrings[1] + "\\right)";
		}
		else {
			std::string out = "\\left(";
			for (size_t i = 0; i < tailStrings.size(); i++) {
				out += tailStrings[i] + " ";
				if (i != tailStrings.size() - 1) out += ",";
			}
			out += "\\right)";
			return out;
		}
	}
	return "Unable to generate string? This should not happen!";
}

std::function<Object* (Tuple*)> generateExpressionImpl(AST::Node* head, std::vector<std::string>* unknowns) {
	if (head->type == AST::NodeTypes::C) {
		Object* constant = ((AST::CNode*)head)->constant;
		return [=](Tuple* t) {
			return constant;
			};
	}
	if (head->type == AST::NodeTypes::U) {
		std::string unknownName = ((AST::UNode*)head)->name;
		size_t getFrom = unknowns->size();
		for (size_t i = 0; i < unknowns->size(); i++) {
			if ((*unknowns)[i] == unknownName) {
				getFrom = i;
				break;
			}
		}
		if (getFrom == unknowns->size()) unknowns->push_back(unknownName);
		return [=](Tuple* t) {
			return (*t)[getFrom];
		};
	}
	std::vector<std::function<Object* (Tuple*)>> tailImpls;
	for (AST::Node* t : head->tail) tailImpls.push_back(generateExpressionImpl(t, unknowns));
	if (head->type == AST::NodeTypes::F) {
		Function* function = ((AST::FNode*)head)->function;
		std::function<Object* (Tuple*)> input = tailImpls[0];
		return [=](Tuple* t) {
			Object* result = input(t);
			if (!SlateDefinitions::objectIsIn(result, function->domain)) throw RuntimeDomainException(head->debugLocation.begin,head->debugLocation.end);
			return function->evaluate(result);
		};
	}
	if (head->type == AST::NodeTypes::J) {
		size_t inputSize = head->tail.size();
		Object** os = new Object * [inputSize];
		Tuple* joined = new Tuple(inputSize, os);
		return [=](Tuple* t) {
			for (size_t i = 0; i < inputSize; i++) (*joined)[i] = tailImpls[i](t);
			return joined;
		};
	}
}

bool hasUnknowns(AST::Node* node) {
	if (node->type == AST::NodeTypes::U) return true;
	for (AST::Node* t : node->tail) {
		if (hasUnknowns(t)) return true;
	}
	return false;
}

Expression* SlateContext::expressionFromAST(AST::Node* head, std::vector<std::string>* unknowns) {
	std::function<Object* (Tuple*)> impl = generateExpressionImpl(head, unknowns);
	Expression* exp = new Expression(unknowns == nullptr ? 0 : unknowns->size(), impl);
	std::string name = displayStringFromAST(head);
	definitions.registerString(exp, name);
	return exp;
}

Object* SlateContext::processSyntaxLine(std::string line) {

	std::vector<Lexer::Token*> tokens;
	Lexer::lexer(line, definitions, tokens);

	AST::Node* output = Parser::parser(tokens);
	AST::Node* outputCopy = AST::copyTree(output);
	AST::Node* simpl = simplifyTree(outputCopy);

	printNode(output);
	std::cout << "\n";
	if (simpl != nullptr) printNode(simpl);

	std::string equals = "=";
	if (output->type == AST::NodeTypes::F && ((AST::FNode*)output)->function == definitions.getDefinition(equals)) {
		AST::Node* tailJ = output->tail[0];
		AST::Node* left = tailJ->tail[0];
		AST::Node* right = tailJ->tail[1];
		if (left->type == AST::NodeTypes::U && !hasUnknowns(right)) {
			Expression* rightExp = expressionFromAST(right, nullptr);
			Object* result = rightExp->evalFunc(nullptr);
			definitions.registerDefinition(((AST::UNode*)left)->name, result);
			return result;
		}
	}
	std::vector<std::string> u;
	return expressionFromAST(output, &u);


	return nullptr;

}

void SlateContext::printNode(AST::Node* n, size_t spaces) {
	for (size_t i = 0; i < spaces; i++) std::cout << "  ";
	switch (n->type) {
	case AST::NodeTypes::F: {
		std::cout << "F Node " << "(" + getObjectName(((AST::FNode*)n)->function) + ")\n";
		break;
	}
	case AST::NodeTypes::J: {
		std::cout << "J Node\n";
		break;
	}
	case AST::NodeTypes::C: {
		std::cout << "C Node " << "(" + getObjectName(((AST::CNode*)n)->constant) + ")\n";
		break;
	}
	case AST::NodeTypes::U: {
		std::cout << "U Node " << "(\"" + ((AST::UNode*)n)->name + "\")\n";
		break;
	}
	case AST::NodeTypes::MARKER: {
		std::cout << "MARKER\n";
		break;
	}
	}
	for (AST::Node* t : n->tail) printNode(t, spaces + 1);
}

bool SlateContext::equalsNodes(SlateLanguage::AST::Node* first, SlateLanguage::AST::Node* second) {
	if (first->type != second->type) return false;
	if (first->tail.size() != second->tail.size()) return false;
	switch (first->type) {
	case AST::NodeTypes::C: {
		AST::CNode* firstC = (AST::CNode*)first;
		AST::CNode* secondC = (AST::CNode*)second;

		// If one of the constants come from a variable which is already defined in the environment
		// Then the nodes should only be considered the same if it is actually the same variable
		if (definitions.objectHasDefinition(firstC->constant) || definitions.objectHasDefinition(secondC->constant)) {
			return firstC->constant == secondC->constant;
		}

		return SlateDefinitions::equals(firstC->constant, secondC->constant);
	}
	case AST::NodeTypes::F: {
		AST::FNode* firstF = (AST::FNode*)first;
		AST::FNode* secondF = (AST::FNode*)second;
		return SlateDefinitions::equals(firstF->function, secondF->function);
	}
	case AST::NodeTypes::U: {
		AST::UNode* firstU = (AST::UNode*)first;
		AST::UNode* secondU = (AST::UNode*)second;
		return firstU->name == secondU->name;
	}
	case AST::NodeTypes::Q:
	case AST::NodeTypes::J:
		return true;
	case AST::NodeTypes::MARKER: {
		AST::Marker* firstM = (AST::Marker*)first;
		AST::Marker* secondM = (AST::Marker*)second;
		return firstM->marker == secondM->marker;
	}
	default:
		return false;
	}
}

bool SlateContext::equalsAST(SlateLanguage::AST::Node* first, SlateLanguage::AST::Node* second) {
	if (!equalsNodes(first, second)) return false;
	for (size_t i = 0; i < first->tail.size(); i++) {
		if (!equalsAST(first->tail[i], second->tail[i])) return false;
	}
	return true;
}

bool SlateContext::superimposePattern(SlateLanguage::AST::Node* head, SlateLanguage::AST::Node* inputPattern, std::unordered_map<std::string, SlateLanguage::AST::Node*>& patternTemplateInputs) {
	if (inputPattern->type == AST::NodeTypes::U) {
		std::string name = ((AST::UNode*)inputPattern)->name;
		if (patternTemplateInputs.find(name) == patternTemplateInputs.end()) {
			patternTemplateInputs[name] = head;
			return true;
		}
		else {
			return equalsAST(patternTemplateInputs[name], head);
		}
	}
	if (!equalsNodes(head, inputPattern)) return false;
	for (size_t i = 0; i < head->tail.size(); i++) {
		if (!superimposePattern(head->tail[i], inputPattern->tail[i], patternTemplateInputs)) return false;
	}
	return true;
}

void SlateContext::populatePattern(SlateLanguage::AST::Node* pattern, std::unordered_map<std::string, SlateLanguage::AST::Node*>& patternTemplateInputs) {
	for (size_t i = 0; i < pattern->tail.size();i++) {
		if (pattern->tail[i]->type == AST::NodeTypes::U) {
			std::string name = ((AST::UNode*)pattern->tail[i])->name;
			if (patternTemplateInputs.find(name) == patternTemplateInputs.end()) throw CompileDidNotUnderstandExpression();
			pattern->tail[i] = patternTemplateInputs[name];
		} else {
			populatePattern(pattern->tail[i], patternTemplateInputs);
		}
	}
}

SlateLanguage::AST::Node* SlateContext::simplifyTree(SlateLanguage::AST::Node* head) {

	for (size_t i = 0; i < head->tail.size();i++) {
		AST::Node* simplified;
		while ((simplified = simplifyTree(head->tail[i])) != nullptr) head->tail[i] = simplified;
	}

	for (Equivalence eq : definitions.equivalences) {
		std::unordered_map<std::string, SlateLanguage::AST::Node*> inputs;
		if (superimposePattern(head, eq.from, inputs)) {
			AST::Node* blankPattern = AST::copyTree(eq.to);
			populatePattern(blankPattern, inputs);
			return blankPattern;
		}
	}

	return nullptr;
}
