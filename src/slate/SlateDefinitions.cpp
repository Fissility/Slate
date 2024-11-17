#include "SlateDefinitions.h"
#include "objects/tuple/BiCategory.h"
#include <fstream>
#include <cmath>

std::vector<std::string> SlateDefinitions::symbolBases;
std::vector<std::string> SlateDefinitions::symbolFlares;
std::vector<std::string> SlateDefinitions::controlSeqeuenceCharacters;
std::vector<std::string> SlateDefinitions::binaryOperators;
std::unordered_map<std::string, std::string> SlateDefinitions::controlSequenceFunctions;

void dumpListToVec(std::string path,std::vector<std::string>& list) {
	std::ifstream f(path);
	std::string name;
	while (std::getline(f, name)) {
		list.push_back(name);
	}
}

void dumpDictToMap(std::string path, std::unordered_map<std::string, std::string>& map) {
	std::ifstream f(path);
	std::string name;
	while (std::getline(f, name)) {
		size_t sep = name.find(';');
		map[name.substr(0, sep)] = name.substr(sep + 1, name.size() - sep - 1);
	}
}

void SlateDefinitions::loadSymbols() {
	dumpListToVec("slate_conf/symbol_base.list", symbolBases);
	dumpListToVec("slate_conf/symbol_flare.list", symbolFlares);
	dumpListToVec("slate_conf/ctrl_seq_characters.list", controlSeqeuenceCharacters);
	dumpListToVec("slate_conf/binary_operators.list", binaryOperators);
	dumpDictToMap("slate_conf/ctrl_seq_function.dict", controlSequenceFunctions);
}

std::string normaliseName(std::string name) {
	std::string normal;
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
}

bool Definitions::objectExists(std::string& name) {
	return definitions.find(normaliseName(name)) != definitions.end();
}

Object* Definitions::getObject(std::string& name) {
	return definitions[normaliseName(name)];
}

void Definitions::registerString(Object* o, std::string string) {
	stringValues[o] = string;
}

bool Definitions::objectHasString(Object* o) {
	return stringValues.find(o) != stringValues.end();
}

std::string Definitions::getString(Object* o) {
	return stringValues[o];
}

void Definitions::registerBaseObject(Object* o, std::string name) {
	registerDefinition(name, o);
	registerString(o, name);
}

void Definitions::clear() {
	definitions.clear();
	stringValues.clear();
}

Definitions SlateDefinitions::buildDefaultDefinitions() {

	Definitions defs;

	Set* AllSets_set = new AllSetsSet();
	Set* AllSets2_set = AllSets_set->cartesian_with(AllSets_set);
	Set* U_set = new USet();
	Set* U2_set = U_set->cartesian_with(U_set);
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

	Set* R2_set = R_set->cartesian_with(R_set);
	defs.registerBaseObject(R2_set, "\\mathbb{R}^2");

	Set* R2Star_set = R_set->cartesian_with(RStar_set);
	defs.registerBaseObject(R2Star_set, "\\mathbb{R}\\times\\mathbb{R}^*");

	Set* RPositive = new IntervalSet(new Number(0), infinity, true, false);

	BinaryOperator* addition = new BinaryOperator(R2_set->union_with(R_set), R_set, [](Object* o) {
		if (o->type == Types::TUPLE) {
			Tuple* t = (Tuple*)o;
			Number* n1 = (Number*)t->objects[0];
			Number* n2 = (Number*)t->objects[1];
			return (Object*)(new Number(*n1 + *n2));
		}
		return (Object*)(new Number(*((Number*)o)));
	},ADDITION);

	addition->hasUnary(true);
	defs.registerBaseObject(addition, "+");

	BinaryOperator* subtraction = new BinaryOperator(R2_set->union_with(R_set), R_set, [](Object* o) {
		if (o->type == Types::TUPLE) {
			Tuple* t = (Tuple*)o;
			Number* n1 = (Number*)t->objects[0];
			Number* n2 = (Number*)t->objects[1];
			return (Object*)(new Number(*n1 - *n2));
		}
		return (Object*)(new Number(-*((Number*)o)));
	},SUBTRACTION);

	subtraction->hasUnary(true);
	defs.registerBaseObject(subtraction, "-");

	BinaryOperator*  multiplication = new BinaryOperator(R2_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		output->value = *n1 * *n2;
		return output;
	},MULTIPLICATION);
	defs.registerBaseObject(multiplication, "\\cdot");

	BinaryOperator* division = new BinaryOperator(R2Star_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		output->value = *n1 / *n2;
		return output;
	}, DIVISION);
	defs.registerBaseObject(division, "\\div");

	Function* division_fraction = new Function(R2Star_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		output->value = *n1 / *n2;
		return output;
	});
	defs.registerBaseObject(division_fraction, "\\frac");

	Function* power = new BinaryOperator(R2_set, R_set, [](Object* o) {
		//static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
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

	return defs;

}
