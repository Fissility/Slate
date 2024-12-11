#include "SlateContext.h"
#include "SlateDefinitions.h"
#include <format>
#include "language/SlateErrors.h"
#include "language/Tokenizer.h"
#include "language/Lexer.h"
#include "language/Parser.h"
#include "objects/Expression.h"
#include "objects/tuple/BiCategory.h" //X
#include <iostream>
#include <stack>

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
				getFrom == i;
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
			return function->evaluate(input(t));
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

	if (output->type == AST::NodeTypes::MARKER && ((AST::Marker*)output)->marker == Lexer::MarkerTypes::EQUALS) {
		AST::Node* left = output->tail[0];
		AST::Node* right = output->tail[1];
		if (left->type == AST::NodeTypes::U && !hasUnknowns(right)) {
			Expression* rightExp = expressionFromAST(right, nullptr);
			Object* result = rightExp->evalFunc(nullptr);
			definitions.registerDefinition(((AST::UNode*)left)->name, result);
			return result;
		}
	}
	else {
		std::vector<std::string> u;
		return expressionFromAST(output, &u);
	}

	return nullptr;

}