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

	Tuple*		join	(Object*	o1, Object*		o2, size_t nestingLevel1, size_t nestingLevel2, size_t currentLevel);
	Known*		join_kk	(Known*		k1, Known*		k2, size_t commaLevel);
	Dependent*	join_uu	(Unknown*	u1, Unknown*	u2, size_t commaLevel);
	Dependent*	join_dd	(Dependent* d1, Dependent*	d2, size_t commaLevel);
	Dependent*	join_uk	(Unknown*	u,	Known*		k,	size_t commaLevel);
	Dependent*	join_ku	(Known*		k,	Unknown*	u,	size_t commaLevel);
	Dependent*	join_ud	(Unknown*	u,	Dependent*	d,	size_t commaLevel);
	Dependent*	join_du	(Dependent* d,	Unknown*	u,	size_t commaLevel);
	Dependent*	join_kd	(Known*		k,	Dependent*	d,	size_t commaLevel);
	Dependent*	join_dk	(Dependent* d,	Known*		k,	size_t commaLevel);
	ObjectSyntaxWrapper* joinObjects(ObjectSyntaxWrapper* left, ObjectSyntaxWrapper* right, size_t commaLevel = SIZE_MAX);

	Known*		func_k(Known* func, Known* k, bool isBinary);
	Dependent*	func_u(Known* func, Unknown* u);
	Dependent*	func_d(Known* func, Dependent* d, bool isBinary);
	ObjectSyntaxWrapper* funcPass(Known* func, ObjectSyntaxWrapper* obj, bool isBinary);

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