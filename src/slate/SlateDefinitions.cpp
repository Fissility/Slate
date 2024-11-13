#include "SlateDefinitions.h"
#include "objects/tuple/BiCategory.h"
#include <fstream>
#include <cmath>

std::vector<std::string> SlateDefinitions::symbolBases;
std::vector<std::string> SlateDefinitions::symbolFlares;
std::vector<std::string> SlateDefinitions::controlSeqeuenceCharacters;
std::vector<std::string> SlateDefinitions::binaryOperators;
std::unordered_map<std::string, std::string> SlateDefinitions::controlSequenceFunctions;

AllSetsSet* SlateDefinitions::AllSets_set;
Set* SlateDefinitions::AllSets2_set;
USet* SlateDefinitions::U_set;
Set* SlateDefinitions::U2_set;
NSet* SlateDefinitions::N_set;
ZSet* SlateDefinitions::Z_set;
QSet* SlateDefinitions::Q_set;
RSet* SlateDefinitions::R_set;
Set* SlateDefinitions::R2_set;
Function* SlateDefinitions::addition_func;
Function* SlateDefinitions::subtraction_func;
Function* SlateDefinitions::multiplication_func;
Function* SlateDefinitions::division_func;
Function* SlateDefinitions::division_fraction_func;
Function* SlateDefinitions::power_func;
Function* SlateDefinitions::sqrt_func;
Function* SlateDefinitions::setCategoryBinding_func;

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

	AllSets_set = new AllSetsSet();
	AllSets2_set = AllSets_set->cartesian_with(AllSets_set);
	U_set = new USet();
	U2_set = U_set->cartesian_with(U_set);
	N_set = new NSet();
	Z_set = new ZSet();
	Q_set = new QSet();
	R_set = new RSet();
	R2_set = R_set->cartesian_with(R_set);

	addition_func = new BinaryOperator(R2_set->union_with(R_set), R_set, [](Object* o) {
		if (o->type == Types::TUPLE) {
			Tuple* t = (Tuple*)o;
			Number* n1 = (Number*)t->objects[0];
			Number* n2 = (Number*)t->objects[1];
			return (Object*)(new Number(*n1 + *n2));
		}
		return (Object*)(new Number(*((Number*)o)));
	},ADDITION);

	((BinaryOperator*)addition_func)->hasUnary(true);

	subtraction_func = new BinaryOperator(R2_set->union_with(R_set), R_set, [](Object* o) {
		//static Number* output = new Number(0);
		if (o->type == Types::TUPLE) {
			Tuple* t = (Tuple*)o;
			Number* n1 = (Number*)t->objects[0];
			Number* n2 = (Number*)t->objects[1];
			return (Object*)(new Number(*n1 - *n2));
		}
		return (Object*)(new Number(-*((Number*)o)));
	},SUBTRACTION);

	((BinaryOperator*)subtraction_func)->hasUnary(true);

	multiplication_func = new BinaryOperator(R2_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		output->value = *n1 * *n2;
		return output;
	},MULTIPLICATION);

	division_func = new BinaryOperator(R2_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		output->value = *n1 / *n2;
		return output;
	}, DIVISION);

	division_fraction_func = new Function(R2_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		output->value = *n1 / *n2;
		return output;
	});

	power_func = new BinaryOperator(R2_set, R_set, [](Object* o) {
		//static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		return new Number(std::pow(*n1,*n2));
	}, POWER);

	sqrt_func = new Function(R_set, R_set, [](Object* o) {
		return new Number(std::sqrt(*((Number*)o)));
	});

	setCategoryBinding_func = new BinaryOperator(AllSets2_set, AllSets_set, [](Object* o) {
		Tuple* t = (Tuple*)o;
		BiCategory* cat = new BiCategory((*t)[0], (*t)[1]);
		return cat;
	}, SET_BINDING);

}
