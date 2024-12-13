#include "Parser.h"
#include <iostream>

#include <functional>
#include "../objects/Expression.h"
#include "SlateErrors.h"

using namespace SlateLanguage::Lexer;
using namespace SlateLanguage::AST;

// TODO, implement precedence function
int precedence(Token* wrapper) {
	if (wrapper->type == TokenTypes::MARKER) {
		switch (((MarkerToken*)wrapper)->mType) {
		case MarkerTypes::COLON:
			return INT_MIN + 1;
		case MarkerTypes::COMMA:
			return INT_MIN + 2;
		default:
			break;
		}
	}
	if (isOperator(wrapper)) {
		KnownToken* k = (KnownToken*)wrapper;
		if (k->kind == KnownKinds::OPERATOR) return INT_MAX;
		if (k->kind == KnownKinds::BINARY_OPERATOR_UNARY) return INT_MAX - 1;
		if (k->kind == KnownKinds::BINARY_OPERATOR) {
			return ((BinaryOperator*)((KnownToken*)wrapper)->o)->precedence;
		}
	}
	return 0;
}

void pushOperatorToOutput(std::vector<Node*>& output, Token* op) {
	if (isFunction(op)) {
		FNode* fnode = new FNode((Function*)((KnownToken*)op)->o);
		fnode->tail.push_back(output.back());
		output.back()->head.push_back(fnode);
		output.pop_back();
		output.push_back(fnode);
	}
	if (isBinaryOperator(op)) {
		if (output.size() < 2) throw CompileFloatingOperator(op->location.begin, op->location.end);
		FNode* fnode = new FNode((Function*)((KnownToken*)op)->o);
		JNode* jnode = new JNode(1);
		Node* right = output.back(); output.pop_back();
		Node* left = output.back(); output.pop_back();
		jnode->tail.push_back(left);
		left->head.push_back(jnode);

		jnode->tail.push_back(right);
		right->head.push_back(jnode);

		fnode->tail.push_back(jnode);
		jnode->head.push_back(fnode);

		output.push_back(fnode);
	}
	if (isComma(op)) {
		if (output.size() < 2) throw CompileFloatingOperator(op->location.begin, op->location.end);
		size_t nestingLevel = op->nestingLevel;
		Node* right = output.back(); output.pop_back();
		Node* left = output.back(); output.pop_back();
		if (left->type == NodeTypes::J && ((JNode*)left)->nestingLevel == nestingLevel) {
			JNode* leftJ = (JNode*)left;
			leftJ->tail.push_back(right);
			right->head.push_back(leftJ);
			output.push_back(leftJ);
		}
		else {
			JNode* jnode = new JNode(nestingLevel);

			jnode->tail.push_back(left);
			left->head.push_back(jnode);
			jnode->tail.push_back(right);
			right->head.push_back(jnode);

			output.push_back(jnode);

		}
	} else if (op->type == TokenTypes::MARKER) {
		Marker* mnode = new Marker(((MarkerToken*)op)->mType);

		Node* right = output.back(); output.pop_back();
		Node* left = output.back(); output.pop_back();
		mnode->tail.push_back(left);
		left->head.push_back(mnode);

		mnode->tail.push_back(right);
		right->head.push_back(mnode);

		output.push_back(mnode);
	}
}

bool isUnaryForm(Token* t) {
	if (t->type != TokenTypes::KNOWN) return false;
	return ((KnownToken*)t)->kind == KnownKinds::BINARY_OPERATOR_UNARY;
}

SlateLanguage::AST::Node* SlateLanguage::Parser::parser(std::vector<Token*>&wrappers) {
	std::vector<Token*> operators;
	std::vector<AST::Node*> output;

	for (Token* object : wrappers) {

		if (isConstant(object)) output.push_back(new AST::CNode(((KnownToken*)object)->o));
		else if (isUnknown(object)) output.push_back(new AST::UNode(((UnknownToken*)object)->name));
		else if (isOperator(object)) {
			while (
				!empty(operators) &&
				isOperator(operators.back()) &&
				!(isUnaryForm(object) && isUnaryForm(operators.back())) &&
				precedence(object) <= precedence(operators.back())
				) {

				pushOperatorToOutput(output, operators.back());
				operators.pop_back();
			}

			operators.push_back(object);
		}
		else if (isOpenBracket(object)) operators.push_back(object);
		else if (isClosedBracket(object)) {
			while (!empty(operators) && !isOpenBracket(operators.back())) {
				pushOperatorToOutput(output, operators.back());
				operators.pop_back();
			}
			if (!empty(operators) && isOpenBracket(operators.back())) {
				operators.pop_back();
			}
		}
	}
	while (!empty(operators)) {
		pushOperatorToOutput(output, operators.back());
		operators.pop_back();
	}
	if (output.size() != 1) throw CompileDidNotUnderstandExpression();
	if (!output.empty()) return output[0];
	return nullptr;
}
