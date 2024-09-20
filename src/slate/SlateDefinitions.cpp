#include "SlateDefinitions.h"
#include <fstream>
#include <cmath>

std::vector<std::string> SlateDefinitions::symbolBases;
std::vector<std::string> SlateDefinitions::symbolFlares;

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

	N_set = new NSet();
	Z_set = new ZSet();
	Q_set = new QSet();
	R_set = new RSet();
	R2_set = R_set->cartesian(R_set);

	addition_func = new Function(R2_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		output->value = *n1 + *n2;
		return output;
	});

	subtraction_func = new Function(R2_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		output->value = *n1 - *n2;
		return output;
	});

	multiplication_func = new Function(R2_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		output->value = *n1 * *n2;
		return output;
	});

	division_func = new Function(R2_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		output->value = *n1 / *n2;
		return output;
	});

	power_func = new Function(R2_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		output->value = std::pow(*n1,*n2);
		return output;
	});

}
