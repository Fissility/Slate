#include "SlateDefinitions.h"
#include "objects/tuple/BiCategory.h"
#include <fstream>
#include <cmath>

std::vector<std::string> SlateDefinitions::symbolBases;
std::vector<std::string> SlateDefinitions::symbolFlares;
std::vector<std::string> SlateDefinitions::specialCharacters;
std::vector<std::string> SlateDefinitions::binaryOperators;

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
Function* SlateDefinitions::power_func;
Function* SlateDefinitions::setCategoryBinding_func;

void dumpListToVec(std::string path,std::vector<std::string>& list) {
	std::ifstream f(path);
	std::string name;
	while (std::getline(f, name)) {
		list.push_back(name);
	}
}

void SlateDefinitions::load() {

	dumpListToVec("slate_conf/symbol_base.list", symbolBases);
	dumpListToVec("slate_conf/symbol_flare.list", symbolFlares);
	dumpListToVec("slate_conf/special_characters.list", specialCharacters);
	dumpListToVec("slate_conf/binary_operators.list", binaryOperators);

	AllSets_set = new AllSetsSet();
	AllSets2_set = AllSets_set->cartesian_with(AllSets_set);
	U_set = new USet();
	U2_set = U_set->cartesian_with(U_set);
	N_set = new NSet();
	Z_set = new ZSet();
	Q_set = new QSet();
	R_set = new RSet();
	R2_set = R_set->cartesian_with(R_set);

	addition_func = new BinaryOperator(R2_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		output->value = *n1 + *n2;
		return output;
	},ADDITION);

	addition_func->addInverse([](InputForm iF,Object* constants[]) {
		if (iF.type != InputFormTypes::MULTIPLE) return (Function*)nullptr;
		if (iF.otherForms->size() != 2) return (Function*)nullptr;
		InputForm& i1 = (*iF.otherForms)[0];
		InputForm& i2 = (*iF.otherForms)[1];
		if (i1.type == InputFormTypes::MULTIPLE || i2.type == InputFormTypes::MULTIPLE) return (Function*)nullptr;
		if (i1.type == InputFormTypes::INPUT && i2.type == InputFormTypes::INPUT) return (Function*)nullptr;
		Object* c = constants[0];
		return new Function(addition_func->codomain, addition_func->domain, [=](Object * o) {
			static Number* output = new Number(0);
			output->value = (Number*)o - (Number*)c;
			return output;
		});
	});

	subtraction_func = new BinaryOperator(R2_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		output->value = *n1 - *n2;
		return output;
	},SUBTRACTION);

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

	setCategoryBinding_func = new BinaryOperator(AllSets2_set, AllSets_set, [](Object* o) {
		static Tuple* tOut = new Tuple(2, new Object*[]{ nullptr,nullptr });
		BiCategory* t = (BiCategory*)o;
		tOut->objects[0] = t->get(0);
		tOut->objects[1] = t->get(1);
		return tOut;
	}, SET_BINDING);

}
