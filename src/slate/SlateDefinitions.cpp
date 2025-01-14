#include "SlateDefinitions.h"
#include "objects/tuple/BiCategory.h"
#include "language/Lexer.h"
#include "language/Parser.h"
#include <cfloat>
#include <cstdio>
#include <fstream>
#include <cmath>
#include <climits>

std::vector<std::string> SlateDefinitions::symbolBases;
std::vector<std::string> SlateDefinitions::symbolFlares;
std::vector<std::string> SlateDefinitions::controlSeqeuenceCharacters;
std::vector<std::string> SlateDefinitions::binaryOperators;
std::unordered_map<std::string, std::string> SlateDefinitions::controlSequenceFunctions;

bool getLine(std::ifstream& stream, std::string& s) {
  if (stream.peek() == EOF) return false;

  s = "";

  bool found = false;

  char c;
  while (stream.peek() != EOF && stream.peek() != '\n' && stream.peek() != '\r') {
    stream.get(c);
    s += c;
    found = true; 
  }
  
  while (stream.peek() != EOF && (stream.peek() == '\n' || stream.peek() == '\r')) stream.get(c);

  return found;
}

void dumpListToVec(std::string path,std::vector<std::string>& list) {
	std::ifstream f(path);
	std::string name;
	while (getLine(f, name)) {
		list.push_back(name);
	}
}

void dumpDictToMap(std::string path, std::unordered_map<std::string, std::string>& map) {
	std::ifstream f(path);
	std::string name;
	while (getLine(f, name)) {
		size_t sep = name.find(';');
		map[name.substr(0, sep)] = name.substr(sep + 1, name.size() - sep - 1);
	}
}

bool SlateDefinitions::equals(Object* first, Object* second) {
	if (first->type != second->type) return false;
	switch (first->type) {
		case Types::NUMBER: {
			return ((*(Number*)(first)) == (*((Number*)second)));
		}
		default: {
			return first == second;
		}
	}
}

bool SlateDefinitions::objectIsIn(Object* o, Set* s) {
	return s->inImpl(o);
}

Set* SlateDefinitions::setUnion(Set* first, Set* second) {
	return new Set(
		[=](Object* o) {
			return objectIsIn(o,first) || objectIsIn(o,second);
		}
	);
}

Set* SlateDefinitions::setIntersection(Set* first, Set* second) {
	return new Set(
		[=](Object* o) {
			return objectIsIn(o,first) && objectIsIn(o,second);
		}
	);
}

Set* SlateDefinitions::setMinus(Set* first, Set* second) {
	return new Set(
		[=](Object* o) {
			return objectIsIn(o,first) && !objectIsIn(o,second);
		}
	);
}

Set* SlateDefinitions::setCartesian(Set* first, Set* second) {
	Set* prod = new Set(
		[=](Object* o) {
			if ((o->type) != Types::TUPLE) return false;
			Tuple* t = (Tuple*)o;
			// For canonical tuple result of the cartesian product
			if (t->length == 2) {
				Object* o1 = t->get(0);
				Object* o2 = t->get(1);
				return objectIsIn(o1,first) && objectIsIn(o2,second);
			}
		}
	);
	return prod;
}

void SlateDefinitions::loadSymbols() {
	dumpListToVec("./slate_conf/symbol_base.list", symbolBases);
	dumpListToVec("./slate_conf/symbol_flare.list", symbolFlares);
	dumpListToVec("./slate_conf/ctrl_seq_characters.list", controlSeqeuenceCharacters);
	dumpListToVec("./slate_conf/binary_operators.list", binaryOperators);
	dumpDictToMap("./slate_conf/ctrl_seq_function.dict", controlSequenceFunctions);
}

std::string replace(std::string in, std::string sequence, std::string replace) {
	while (in.find(sequence) != -1) {
		in.replace(in.find(sequence), sequence.size(), replace);
	}
	return in;
}

std::string normaliseName(std::string name) {
	std::string normal;
	name = replace(name, "\\left", "");
	name = replace(name, "\\right", "");
	bool lastWasBackS = false;
	for (size_t i = 0; i < name.size(); i++) {
		char c = name[i];
		switch (c) {
		case '{':
		case '}': {
			if (lastWasBackS) {
				normal += c;
				lastWasBackS = false;
			}
			break;
		}
		case ' ':
			break;
		case '\\': {
			lastWasBackS = true;
			normal += c;
			break;
		}
		default: {
			lastWasBackS = false;
			normal += c;
		}
		}
	}
	return normal;
}

void Definitions::registerDefinition(std::string& name, Object* o) {
	definitions[normaliseName(name)] = o;
	definedObjects.push_back(o);
}

bool Definitions::definitionExists(std::string& name) {
	return definitions.find(normaliseName(name)) != definitions.end();
}

bool Definitions::objectHasDefinition(Object* o) {
	return std::find(definedObjects.begin(),definedObjects.end(),o) != definedObjects.end();
}

Object* Definitions::getDefinition(std::string& name) {
	return definitions[normaliseName(name)];
}

void Definitions::registerString(Object* o, std::string string) {
	stringValues[o] = string;
}

bool Definitions::objectHasString(Object* o) {
	return stringValues.find(o) != stringValues.end();
}

std::string Definitions::getString(Object* o) {
	if (!objectHasString(o)) return "\\mathrm{[Object cannot be displayed]}";
	return stringValues[o];
}

void Definitions::registerBaseObject(Object* o, std::string name) {
	registerDefinition(name, o);
	registerString(o, name);
}

void Definitions::registerEquivalence(Equivalence eq) {
	equivalences.push_back(eq);
}

void Definitions::clear() {
	definitions.clear();
	stringValues.clear();
}

Equivalence generateEquivalence(std::string from, std::string to, Definitions& defs) {
	std::vector<SlateLanguage::Lexer::Token*> tokens;
	SlateLanguage::Lexer::lexer(from, defs, tokens);
	SlateLanguage::AST::Node* nodeFrom = SlateLanguage::Parser::parser(tokens);
	tokens.clear();
	SlateLanguage::Lexer::lexer(to, defs, tokens);
	SlateLanguage::AST::Node* nodeTo = SlateLanguage::Parser::parser(tokens);
	return Equivalence(nodeFrom, nodeTo);
}

Definitions SlateDefinitions::buildDefaultDefinitions() {

	Definitions defs;

	Set* AllSets_set = new AllSetsSet();
	Set* AllSets2_set = setCartesian(AllSets_set, AllSets_set);
	Set* U_set = new USet();
	Set* U2_set = setCartesian(U_set, U_set);
	Number* infinity = new Number(DBL_MAX);

	Set* N_set = new NSet();
	defs.registerBaseObject(N_set, "\\mathbb{N}");
	Set* NStar_set = new NStarSet();
	defs.registerBaseObject(NStar_set, "\\mathbb{N}^*");

	Set* Z_set = new ZSet();
	defs.registerBaseObject(Z_set, "\\mathbb{Z}");
	Set* ZStar_set = new ZStarSet();
	defs.registerBaseObject(ZStar_set, "\\mathbb{Z}^*");

	Set* Q_set = new QSet();
	defs.registerBaseObject(Q_set, "\\mathbb{Q}");
	Set* QStar_set = new QStarSet();
	defs.registerBaseObject(QStar_set, "\\mathbb{Q}^*");

	Set* R_set = new RSet();
	defs.registerBaseObject(R_set, "\\mathbb{R}");
	Set* RStar_set = new RStarSet();
	defs.registerBaseObject(RStar_set, "\\mathbb{R}^*");

	Set* R2_set = setCartesian(R_set, R_set);
	defs.registerBaseObject(R2_set, "\\mathbb{R}^2");

	Set* R2Star_set = setCartesian(R_set, RStar_set);
	defs.registerBaseObject(R2Star_set, "\\mathbb{R}\\times\\mathbb{R}^*");

	Set* RPositive = new IntervalSet(new Number(0), infinity, true, false);

	Set* B_set = new BSet();

	BinaryOperator* equalsFunc = new BinaryOperator(U2_set, B_set, [](Object* o) {
		static Number* FALSE = new Number(0);
		static Number* TRUE = new Number(1);
		Tuple* t = (Tuple*)o;
		Object* first = (*t)[0];
		Object* second = (*t)[1];
		if (equals(first, second)) return TRUE;
		else return FALSE;
	},EQUALS);

	defs.registerBaseObject(equalsFunc, "=");

	BinaryOperator* addition = new BinaryOperator(setUnion(R2_set,R_set), R_set, [](Object* o) {
		if (o->type == Types::TUPLE) {
			Tuple* t = (Tuple*)o;
			Number* n1 = (Number*)(*t)[0];
			Number* n2 = (Number*)(*t)[1];
			return (Object*)(new Number(*n1 + *n2));
		}
		return (Object*)(new Number(*((Number*)o)));
	},ADDITION);

	addition->hasUnary(true);
	defs.registerBaseObject(addition, "+");

	BinaryOperator* subtraction = new BinaryOperator(setUnion(R2_set,R_set), R_set, [](Object* o) {
		if (o->type == Types::TUPLE) {
			Tuple* t = (Tuple*)o;
			Number* n1 = (Number*)(*t)[0];
			Number* n2 = (Number*)(*t)[1];
			return (Object*)(new Number(*n1 - *n2));
		}
		return (Object*)(new Number(-*((Number*)o)));
	},SUBTRACTION);

	subtraction->hasUnary(true);
	defs.registerBaseObject(subtraction, "-");

	BinaryOperator*  multiplication = new BinaryOperator(R2_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)(*t)[0];
		Number* n2 = (Number*)(*t)[1];
		output->value = *n1 * *n2;
		return output;
	},MULTIPLICATION);
	defs.registerBaseObject(multiplication, "\\cdot");

	BinaryOperator* division = new BinaryOperator(R2Star_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)(*t)[0];
		Number* n2 = (Number*)(*t)[1];
		output->value = *n1 / *n2;
		return output;
	}, DIVISION);
	defs.registerBaseObject(division, "\\div");

	Function* division_fraction = new Function(R2Star_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)(*t)[0];
		Number* n2 = (Number*)(*t)[1];
		output->value = *n1 / *n2;
		return output;
	});
	defs.registerBaseObject(division_fraction, "\\frac");

	Function* power = new BinaryOperator(R2_set, R_set, [](Object* o) {
		//static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)(*t)[0];
		Number* n2 = (Number*)(*t)[1];
		return new Number(std::pow(*n1,*n2));
	}, POWER);
	defs.registerBaseObject(power, "^");

	Function*  sqrt = new Function(RPositive, R_set, [](Object* o) {
		return new Number(std::sqrt(*((Number*)o)));
	});
	defs.registerBaseObject(sqrt, "\\sqrt");

	Function* setCategoryBinding_func = new BinaryOperator(AllSets2_set, AllSets_set, [](Object* o) {
		Tuple* t = (Tuple*)o;
		BiCategory* cat = new BiCategory((*t)[0], (*t)[1]);
		return cat;
	}, SET_BINDING);

	std::vector<std::string> simplifications;
	dumpListToVec("slate_conf/default_simplifications.list", simplifications);
	for (size_t i = 0; i < simplifications.size(); i++) {
		std::vector<SlateLanguage::Lexer::Token*> tokens;
		SlateLanguage::Lexer::lexer(simplifications[i], defs, tokens);
		SlateLanguage::AST::Node* simplification = SlateLanguage::Parser::parser(tokens);
		if (simplification->type == SlateLanguage::AST::NodeTypes::F && 
			((SlateLanguage::AST::FNode*)simplification)->function == equalsFunc) {
			SlateLanguage::AST::Node* from = simplification->tail[0]->tail[0];
			SlateLanguage::AST::Node* to = simplification->tail[0]->tail[1];
			defs.registerEquivalence({ from,to });
		}
	}

	return defs;

}
