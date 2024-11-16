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
std::unordered_map<Object*, std::string> SlateDefinitions::defaultNames;

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

void registerObject(Object* o, std::string name) {
	SlateDefinitions::defaultDefinitions[normaliseName(name)] = o;
	SlateDefinitions::defaultNames[o] = name;
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
	Number* infinity = new Number(DBL_MAX);

	Set* N_set = new NSet();
	registerObject(N_set, "\\mathbb{N}");
	Set* NStar_set = new NStarSet();
	registerObject(NStar_set, "\\mathbb{N}^*");

	Set* Z_set = new ZSet();
	registerObject(Z_set, "\\mathbb{Z}");
	Set* ZStar_set = new ZStarSet();
	registerObject(ZStar_set, "\\mathbb{Z}^*");

	Set* Q_set = new QSet();
	registerObject(Q_set, "\\mathbb{Q}");
	Set* QStar_set = new QStarSet();
	registerObject(QStar_set, "\\mathbb{Q}^*");

	Set* R_set = new RSet();
	registerObject(R_set, "\\mathbb{R}");
	Set* RStar_set = new RStarSet();
	registerObject(RStar_set, "\\mathbb{R}^*");

	Set* R2_set = R_set->cartesian_with(R_set);
	defaultNames[R2_set] = "\\mathbb{R}^2";
	registerObject(R2_set, "\\mathbb{R}^2");

	Set* R2Star_set = R_set->cartesian_with(RStar_set);
	defaultNames[R2Star_set] = "\\mathbb{R}\\times\\mathbb{R}^*";
	registerObject(R2Star_set, "\\mathbb{R}\\times\\mathbb{R}^*");

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
	registerObject(addition, "+");

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
	registerObject(subtraction, "-");

	BinaryOperator*  multiplication = new BinaryOperator(R2_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		output->value = *n1 * *n2;
		return output;
	},MULTIPLICATION);
	registerObject(multiplication, "\\cdot");

	BinaryOperator* division = new BinaryOperator(RStar_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		output->value = *n1 / *n2;
		return output;
	}, DIVISION);
	registerObject(division, "\\div");

	Function* division_fraction = new Function(RStar_set, R_set, [](Object* o) {
		static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		output->value = *n1 / *n2;
		return output;
	});
	registerObject(division_fraction, "\\frac");

	Function* power = new BinaryOperator(R2_set, R_set, [](Object* o) {
		//static Number* output = new Number(0);
		Tuple* t = (Tuple*)o;
		Number* n1 = (Number*)t->objects[0];
		Number* n2 = (Number*)t->objects[1];
		return new Number(std::pow(*n1,*n2));
	}, POWER);
	registerObject(power, "^");

	Function*  sqrt = new Function(RPositive, R_set, [](Object* o) {
		return new Number(std::sqrt(*((Number*)o)));
	});
	defaultDefinitions["\\sqrt"] = sqrt;
	registerObject(sqrt, "\\sqrt");

	Function* setCategoryBinding_func = new BinaryOperator(AllSets2_set, AllSets_set, [](Object* o) {
		Tuple* t = (Tuple*)o;
		BiCategory* cat = new BiCategory((*t)[0], (*t)[1]);
		return cat;
	}, SET_BINDING);

}
