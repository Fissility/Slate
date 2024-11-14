#include "SlateDefinitions.h"
#include "objects/tuple/BiCategory.h"
#include <fstream>
#include <cmath>

std::vector<std::string> SlateDefinitions::symbolBases;
std::vector<std::string> SlateDefinitions::symbolFlares;
std::vector<std::string> SlateDefinitions::controlSeqeuenceCharacters;
std::vector<std::string> SlateDefinitions::binaryOperators;
std::unordered_map<std::string, std::string> SlateDefinitions::controlSequenceFunctions;

std::unordered_map<std::string, Object*> SlateDefinitions::defaultDefinitions;

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

void SlateDefinitions::load() {

	dumpListToVec("slate_conf/symbol_base.list", symbolBases);
	dumpListToVec("slate_conf/symbol_flare.list", symbolFlares);
	dumpListToVec("slate_conf/ctrl_seq_characters.list", controlSeqeuenceCharacters);
	dumpListToVec("slate_conf/binary_operators.list", binaryOperators);
	dumpDictToMap("slate_conf/ctrl_seq_function.dict", controlSequenceFunctions);

	Set* AllSets_set = new AllSetsSet();
	Set* AllSets2_set = AllSets_set->cartesian_with(AllSets_set);
	Set* U_set = new USet();
	Set* U2_set = U_set->cartesian_with(U_set);
	Set* N_set = new NSet();
	Set* Z_set = new ZSet();
	Set* Q_set = new QSet();
	Set* R_set = new RSet();
	Set* R2_set = R_set->cartesian_with(R_set);

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
	defaultDefinitions["+"] = addition;

	BinaryOperator* subtraction = new BinaryOperator(R2_set->union_with(R_set), R_set, [](Object* o) {
		//static Number* output = new Number(0);
		if (o->type == Types::TUPLE) {
			Tuple* t = (Tuple*)o;
			Number* n1 = (Number*)t->objects[0];
			Number* n2 = (Number*)t->objects[1];
			return (Object*)(new Number(*n1 - *n2));
		}
		return (Object*)(new Number(-*((Number*)o)));
	},SUBTRACTION);

	subtraction->hasUnary(true);
	defaultDefinitions["-"] = subtraction;

	BinaryOperator*  multiplication = new BinaryOperator(R2_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		output->value = *n1 * *n2;
		return output;
	},MULTIPLICATION);
	defaultDefinitions["\\cdot"] = multiplication;

	BinaryOperator* division = new BinaryOperator(R2_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		output->value = *n1 / *n2;
		return output;
	}, DIVISION);
	defaultDefinitions["\\div"] = division;

	Function* division_fraction = new Function(R2_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		output->value = *n1 / *n2;
		return output;
	});
	defaultDefinitions["\\frac"] = division_fraction;

	Function* power = new BinaryOperator(R2_set, R_set, [](Object* o) {
		//static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		return new Number(std::pow(*n1,*n2));
	}, POWER);
	defaultDefinitions["^"] = power;

	Function*  sqrt = new Function(R_set, R_set, [](Object* o) {
		return new Number(std::sqrt(*((Number*)o)));
	});
	defaultDefinitions["\\sqrt"] = sqrt;

	Function* setCategoryBinding_func = new BinaryOperator(AllSets2_set, AllSets_set, [](Object* o) {
		Tuple* t = (Tuple*)o;
		BiCategory* cat = new BiCategory((*t)[0], (*t)[1]);
		return cat;
	}, SET_BINDING);

}
