#include "SlateContext.h"
#include "ParseState.h"
#include "SlateDefinitions.h"
#include <format>
#include "SlateErrors.h"
#include <iostream>
#include <stack>

// ======================================================

bool isAtoB(char c) {
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

bool isSpecialCharacters(std::string s) {
	return std::find(SlateDefinitions::specialCharacters.begin(), SlateDefinitions::specialCharacters.end(), s) != SlateDefinitions::specialCharacters.end();
}

std::string getNextTeX(std::string& s, size_t& i) {
	if (s[i] != '\\') return "";
	std::string name = "\\";
	i++;
	if (isSpecialCharacters(std::string(1, s[i])) && i < s.size()) {
		name += s[i];
		return name;
	}
	while (true) {
		if (isAtoB(s[i]) && i < s.size()) name += s[i];
		else {
			i--;
			break;
		}
		i++;
	}
	return name;
}

void skipWhiteSpaces(std::string& s, size_t& i) {
	while (s[i] == ' ' && i < s.size()) i++;
}

void skipBackWhiteSpaces(std::string& s, size_t& i) {
	if (i > 0 && s[i - 1] == ' ') {
		while (i > 0 && s[i] == ' ') i--;
	}
}

bool isSymbolBase(std::string s) {
	return std::find(SlateDefinitions::symbolBases.begin(), SlateDefinitions::symbolBases.end(), s) != SlateDefinitions::symbolBases.end();
}

bool isSymbolFlare(std::string s) {
	return std::find(SlateDefinitions::symbolFlares.begin(), SlateDefinitions::symbolFlares.end(), s) != SlateDefinitions::symbolFlares.end();
}

bool isBinaryOperator(std::string s) {
	return std::find(SlateDefinitions::binaryOperators.begin(), SlateDefinitions::binaryOperators.end(), s) != SlateDefinitions::binaryOperators.end();
}

size_t jump(std::string& s, size_t& index) {
	skipWhiteSpaces(s, index);
	size_t begin = index;
	if (s[index] != '\\') {
		index++;
	}
	else {
		std::string result = getNextTeX(s, index);
		index++;
	}
	return begin;
}

std::string peek(std::string& s, size_t index) {
	skipWhiteSpaces(s, index);
	if (index >= s.size()) return "";
	if (s[index] != '\\') {
		std::string result(1, s[index]);
		return result;
	}
	std::string result = getNextTeX(s, index);
	return result;
}

bool isEnd(std::string& line, size_t& i) {
	return peek(line, i).size() == 0;
}

/*
* @brief Used to skip and find the end of a curly bracket block
* @param line = The string which contains the text information
* @param start = The starting index of the opening curly bracket
*/
bool iterateOverBrackets(std::string& line, size_t& start) {
	size_t openCount = 1;
	char s;
	bool ignoreNext = false; // Used to ignore things like \{ \} which represent the actual characters and now TeX syntax
	while (openCount != 0) {
		s = line[++start];
		if (s == '{' && !ignoreNext) openCount++;
		if (s == '}' && !ignoreNext) openCount--;
		ignoreNext = false;
		if (s == '\\') ignoreNext = true;
		if (start >= line.size()) return false;
	}
	start++;
	return true;
}

// gotta love TeX
SlateError processSubscript(std::string& line, size_t& i, size_t begin) {
	if (isEnd(line, i)) return EMPTY_SUBSCRIPT(begin,i);
	if (peek(line, i) == "{") {
		begin = jump(line, i);
		if (isEnd(line, i)) return UNCLOSED_SUBSCRIPT(begin, i);
		if (peek(line, i) == "}") return EMPTY_SUBSCRIPT(begin, i);
		if (!iterateOverBrackets(line, i)) return UNCLOSED_SUBSCRIPT(begin, i);
	}
	else if (peek(line, i) == "\\left") {
		begin = jump(line, i);
		bool passedRight = false;
		bool finished = false;
		while (!finished) {
			if (isEnd(line, i)) return UNCLOSED_SUBSCRIPT(begin, i);
			if (passedRight) {
				if (peek(line, i) == ")") finished = true;
				else return OUT_OF_PLACE(begin, i);
			}
			if (peek(line, i) == "\\right") passedRight = true;
			begin = jump(line, i);
		}
	}
	else {
		if (isSymbolFlare(peek(line, i))) {
			begin = jump(line, i);
			if (isEnd(line, i)) return EMPTY_FLARE(begin, i);
			if (peek(line, i) == "{") {
				begin = jump(line, i);
				if (isEnd(line, i)) return UNCLOSED_FLARE(begin, i);
				if (peek(line, i) == "}") return EMPTY_FLARE(begin, i);
				if (!iterateOverBrackets(line, i)) return UNCLOSED_FLARE(begin, i);
			}
			else {
				if (isEnd(line, i)) return EMPTY_FLARE(begin, i);
				begin = jump(line, i);
			}
		}
		else {
			if (isEnd(line, i)) return EMPTY_SUBSCRIPT(begin, i);
			jump(line, i);
		}
	}
	return OK;
}

// ======================================================

SlateContext::SlateContext() {
	nameMap["\\mathbbN"] = SlateDefinitions::N_set;
	nameMap["\\mathbbZ"] = SlateDefinitions::Z_set;
	nameMap["\\mathbbQ"] = SlateDefinitions::Q_set;
	nameMap["\\mathbbR"] = SlateDefinitions::R_set;
	nameMap["+"] = SlateDefinitions::addition_func;
	nameMap["-"] = SlateDefinitions::subtraction_func;
	nameMap["\\div"] = SlateDefinitions::division_func;
	nameMap["\\cdot"] = SlateDefinitions::multiplication_func;
	nameMap["\\Rightarrow"] = SlateDefinitions::setCategoryBinding_func;
	nameMap[","] = SlateDefinitions::tupleBind_func;
}

bool SlateContext::nameExists(std::string name) {
	return nameMap.find(name) != nameMap.end();
}

ExpressionInfo SlateContext::newExpression() {
	expresions.push_back(new std::string(""));
	return ExpressionInfo( *(expresions[expresions.size() - 1]), expresions.size() - 1 );
}

void SlateContext::processSyntax() {
	for (std::string* s : expresions) {
		processSyntaxLine(*s);
	}
}

SlateError SlateContext::processSyntaxLine(std::string& line) {

	SlateError err = OK;

	std::vector<Token> tokens;
	err = lexer(line, tokens);
	if (err.id != 0) return err;

	std::vector<ObjectSyntaxWrapper*> wrappers;
	err = linkTokensToObjects(line, tokens, wrappers);
	if (err.id != 0) return err;

	std::vector<ObjectSyntaxWrapper*> syOut;
	err = shuntingYard(wrappers, syOut);
	if (err.id != 0) return err;

	return OK;

}

SlateError SlateContext::lexer(std::string& line, std::vector<Token>& tokens) {
	size_t begin = 0;
	size_t end = 0;
	size_t i = 0;

	bool flags[2] = { false };

	while(!isEnd(line,i)) {
		std::string current = peek(line, i);

		if (current == "{") {
			jump(line, i);
			continue;
		}

		if (current == "}") {
			if (flags[LexerFlags::FRACTION_OPEN_TOP]) {
				begin = jump(line, i);
				if (isEnd(line, i)) return UNCLOSED_FRACTION(begin,end);
				if (peek(line, i) == "{") {
					jump(line, i);
					end = i;
					tokens.push_back(Token(TokenTypes::FRACTION_BEGIN_SECOND, begin, end));
					flags[LexerFlags::FRACTION_OPEN_TOP] = false;
					flags[LexerFlags::FRACTION_OPEN_BOTTOM] = true;
				}
				else return OUT_OF_PLACE(begin, end);
			}
			else if (flags[LexerFlags::FRACTION_OPEN_BOTTOM]) {
				begin = jump(line, i);
				end = i;
				tokens.push_back(Token(TokenTypes::FRACTION_END, begin, end));
				flags[LexerFlags::FRACTION_OPEN_BOTTOM] = false;
			}
			else {
				jump(line, i);
			}
			continue;
		}

		// CHECK FOR SYMBOL START
		if (isSymbolBase(current)) {
			begin = jump(line, i);
			if (peek(line, i) == "_") {
				jump(line, i);
				end = i;
				SlateError err = processSubscript(line, i, begin);
				end = i;
				if (err.id != 0) return err;
			}
			else end = i;

			tokens.push_back(Token(TokenTypes::SYMBOL, begin, end));
			continue;
		}

		// CHECK FOR FLARE START
		if (isSymbolFlare(current)) {
			begin = jump(line, i);
			if (isEnd(line, i)) return EMPTY_FLARE(begin, end);
			if (peek(line, i) == "{") {
				jump(line, i);
				end = i;
				if (isEnd(line, i)) return UNCLOSED_FLARE(begin, end);
				if (peek(line, i) == "}") return EMPTY_FLARE(begin, end);
				if (!iterateOverBrackets(line, i)) return UNCLOSED_FLARE(begin, end);
				end = i;
			}
			else {
				if (isEnd(line, i)) return EMPTY_FLARE(begin, end);
				jump(line, i);
				end = i;
			}
			if (peek(line, i) == "_") {
				jump(line, i);
				end = i;
				SlateError err = processSubscript(line, i, begin);
				end = i;
				if (err.id != 0) return err;
			}
			tokens.push_back(Token(TokenTypes::SYMBOL, begin, end));
			continue;
		}

		if (isBinaryOperator(current)) {
			begin = jump(line, i);
			end = i;
			tokens.push_back(Token(TokenTypes::OPERATOR, begin, end));
			continue;
		}

		// OPEN BRACKET CHECK
		if (current == "(") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(Token(TokenTypes::BEGIN_SCOPE, begin, end));
			continue;
		}


		// CLOSED BRACKET CHECK
		if (current == ")") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(Token(TokenTypes::END_SCOPE, begin, end));
			continue;
		}

		if (current == "\\frac") {
			begin = jump(line, i);
			if (isEnd(line, i)) return UNCLOSED_FRACTION(begin, end);
			if (peek(line, i) == "{") {
				jump(line, i);
				end = i;
				tokens.push_back(Token(TokenTypes::FRACTION_BEGIN_FIRST, begin, end));
				flags[LexerFlags::FRACTION_OPEN_TOP] = true;
			}
			else return UNCLOSED_FRACTION(begin, end);
			continue;
		}

		return OUT_OF_PLACE(begin, end);
	}
	return OK;
}

std::string normaliseName(std::string name) {
	std::string normal;
	bool lastWasBackS = false;
	for (size_t i = 0; i < name.size(); i++) {
		char c = name[i];
		switch (c) {
			case '{':
			case '}':
				if (lastWasBackS) {
					normal += c;
					lastWasBackS = false;
				}
				break;
			case ' ' :
				break;
			case '\\':
				lastWasBackS = true;
				normal += c;
				break;
			default:
				lastWasBackS = false;
				normal += c;
		}
	}
	return normal;
}

// TODO, implement isOperator
bool isOperator(ObjectSyntaxWrapper* wrapper) {
	if (wrapper->type == SyntaxWrapperTypes::MARKER) {
		if (wrapper->type == MarkerTypes::EQUALS || wrapper->type == MarkerTypes::COLON) return true;
	}
	if (wrapper->type == SyntaxWrapperTypes::KNOWN) {
		Known* i = (Known*)wrapper;
		return i->kind == KnownKinds::OPERATOR || i->kind == KnownKinds::BINARY_OPERATOR;
	}
	return false;
}

bool isOpenBracket(ObjectSyntaxWrapper* wrapper) {
	return wrapper->type == SyntaxWrapperTypes::MARKER && ((Marker*)wrapper)->type == MarkerTypes::BEGIN_SCOPE;
}

bool isClosedBracket(ObjectSyntaxWrapper* wrapper) {
	return wrapper->type == SyntaxWrapperTypes::MARKER && ((Marker*)wrapper)->type == MarkerTypes::END_SCOPE;
}

// TODO, implement isOperand
bool isOperand(ObjectSyntaxWrapper* wrapper) {
	return !isOperator(wrapper) && !isOpenBracket(wrapper) && !isClosedBracket(wrapper);
}

/*
* Links names to defined objects and classified them based on their role in the expression
*/
SlateError SlateContext::linkTokensToObjects(std::string line,std::vector<Token>& tokens, std::vector<ObjectSyntaxWrapper*>& objects) {
	for (Token token : tokens) {
		std::string name = normaliseName(line.substr(token.location.begin, token.location.end-token.location.begin));
		switch (token.type) {
			case TokenTypes::SYMBOL: {
				if (nameExists(name)) {
					Object* o = nameMap[name];
					Known* k = new Known(nameMap[name], KnownKinds::OPERAND);

					k->addDebugLocation(token.location);

					objects.push_back(k);
				}
				else {
					objects.push_back(new Unknown(name, token.location));
				}
				break;
			}
			case TokenTypes::OPERATOR: {
				if (name == "=") {
					objects.push_back(new Marker(MarkerTypes::EQUALS, &token));

				}
				else if (name == ":") {
					objects.push_back(new Marker(MarkerTypes::COLON, &token));
					break;
				}
				else if (nameExists(name)) {

					KnownKind kind;

					if (objects.empty()) kind = KnownKinds::OPERAND; // Means it is at the start, then it should be an operand
					else if (isClosedBracket(objects.back()) || !isOperand(objects.back())) kind = KnownKinds::OPERAND; // Means it is after an operator which means it is an operand
					else kind = KnownKinds::BINARY_OPERATOR; // It means it is an operator

					Known* k = new Known(nameMap[name], kind);

					k->addDebugLocation(token.location);

					objects.push_back(new Known(nameMap[name], kind));
				}
				else return OPERATOR_NOT_DEFINED(token.location.begin, token.location.end);
				break;
			}
			case TokenTypes::BEGIN_SCOPE: {

				// This means if the last thing was a function mark that function as on operator
				// If a function is not followed by ( as in, f(..) then it is assumed that the function is not evaluated here
				// and instead it is treated as an operand, for example f+g, f-g, f \circ g
				if (!objects.empty()) {
					ObjectSyntaxWrapper* ow = objects.back();
					if (ow->type == SyntaxWrapperTypes::KNOWN) {
						Known* ind = (Known*)ow;
						Object* o = ind->o;
						if (o->type == Types::FUNCTION) {
							ind->kind = KnownKinds::OPERATOR;
						}
					}
				}

				objects.push_back(new Marker(MarkerTypes::BEGIN_SCOPE, &token));
				break;
			}
			case TokenTypes::END_SCOPE: {
				objects.push_back(new Marker(MarkerTypes::END_SCOPE, &token));
				break;
			}

		}
	}
	return OK;
}

// TODO, implement precedence function
int precedence(ObjectSyntaxWrapper* wrapper) {
	if (wrapper->type == SyntaxWrapperTypes::MARKER) {
		switch (((Marker*)wrapper)->mType) {
			case MarkerTypes::EQUALS:
				return INT_MIN;
			case MarkerTypes::COLON:
				return INT_MIN + 1;
			default:
				break;
		}
	}
	if (isOperand(wrapper)) {
		return ((BinaryOperator*)wrapper)->precedence;
	}
	return 0;
}

SlateError shuntingYard(std::vector<ObjectSyntaxWrapper*>& wrappers, std::vector<ObjectSyntaxWrapper*>& output) {

	std::vector<ObjectSyntaxWrapper*> operators;

	for (ObjectSyntaxWrapper* object : wrappers) {

		if (isOperand(object)) {

			output.push_back(object);

		}
		else if (isOperator(object)) {

			while (
				!empty(operators) &&
				isOperator(operators.back()) &&
				precedence(object) <= precedence(operators.back())
			) {

				output.push_back(operators.back());
				operators.pop_back();
			}

			operators.push_back(object);

		}
		else if (isOpenBracket(object)) {

			operators.push_back(object);

		}
		else if (isClosedBracket(object)) {

			while (!empty(operators) && !isOpenBracket(operators.back())) {
				output.push_back(operators.back());
				operators.pop_back();
			}
			if (!empty(operators) && isOpenBracket(operators.back())) {
				operators.pop_back();
				if (!operators.empty() && operators.back()->type == SyntaxWrapperTypes::KNOWN) {
					Known* i = (Known*)operators.back();
					if (i->kind == KnownKinds::OPERAND) {
						output.push_back(operators.back());
						operators.pop_back();
					}
				}
			}
		}
	}
	while (!empty(operators)) {
		output.push_back(operators.back());
		operators.pop_back();
	}

	return OK;
}

// (x*4+y)*z
// f(x) = *(4,x)
// 

Dependent* kod(Known* k, Known* op, Dependent* d) {
	size_t ukn = d->uknownsCount();

	Object* o_k = k->o;
	BinaryOperator* o_op = (BinaryOperator*)op->o;
	Function* f_d = d->o;

	Tuple* holder = new Tuple(2);
	(*holder)[0] = o_k;
	Function* wrapper = new Function(f_d->domain, f_d->codomain, [=](Object* o) {
		(*holder)[1] = f_d->evaluate(o);
		// TYPE CHECK FOR o_op
		return o_op->evaluate(holder);
	});
	wrapper->ownedMemeory.push_back(holder);

	Dependent* newd = new Dependent(wrapper, op->locations[0]);

	newd->addDebugLocationsFrom(k);
	newd->addDebugLocationsFrom(d);

	return newd;
}

Dependent* dok(Dependent* d, Known* op, Known* k) {

	Object* o_k = k->o;
	BinaryOperator* o_op = (BinaryOperator*)op->o;
	Function* f_d = d->o;

	Tuple* holder = new Tuple(2);
	(*holder)[0] = o_k;
	Function* wrapper = new Function(f_d->domain, o_op->codomain, [=](Object* o) {
		(*holder)[1] = f_d->evaluate(o);
		// TYPE CHECK FOR o_op
		return o_op->evaluate(holder);
		});
	wrapper->ownedMemeory.push_back(holder);

	Dependent* newd = new Dependent(wrapper, op->locations[0]);

	newd->addDebugLocationsFrom(k);
	newd->addDebugLocationsFrom(d);

	return newd;
}

Dependent* dod(Dependent* dl, Known* op, Dependent* dr) {
	size_t unknownsCountl = dl->uknownsCount();
	size_t unknownsCountr = dr->uknownsCount();
	size_t totalUnknowns = unknownsCountl + unknownsCountr;

	// D(x,y)

	Tuple* holder = new Tuple(totalUnknowns);
	Function* wrapper = new Function();
}

SlateError SlateContext::parser(std::vector<ObjectSyntaxWrapper*>& wrappers) {
	// TODO: function composition function

	bool done = false;

	while (!done) {
		for (size_t i = 0; i < wrappers.size(); i++) {

			ObjectSyntaxWrapper* ow = wrappers[i];

			if (ow->type == SyntaxWrapperTypes::KNOWN) {
				Known* iow = (Known*)ow;

				if (iow->kind == KnownKinds::BINARY_OPERATOR) {
					if (wrappers.size() < 2) return FLOATING_OPERATOR(ow->assosciatedToken->location.begin, ow->assosciatedToken->location.end);

					SyntaxWrapperType t1 = wrappers[i - 1]->type;
					SyntaxWrapperType t2 = wrappers[i - 2]->type;

					if (t1 == SyntaxWrapperTypes::KNOWN && t2 == SyntaxWrapperTypes::KNOWN) {
						Known* i1 = (Known*)t1;

						Known* i2 = (Known*)t2;
						if (i1->kind != KnownKinds::OPERAND || i2->kind != KnownKinds::OPERAND) return FLOATING_OPERATOR(ow->assosciatedToken->location.begin, ow->assosciatedToken->location.end);
						BinaryOperator* bo = (BinaryOperator*)iow->o;
						Tuple t(2, new Object * [] {i1->o,i2->o});
						if (!bo->domain->in(&t)) return DOMAIN_EXCEPTION(ow->assosciatedToken->location.begin, ow->assosciatedToken->location.end);
						Object* result = bo->evaluate(&t);

						delete wrappers[i];
						delete wrappers[i - 1];
						delete wrappers[i - 2];
						Known* k = new Known(result,KnownKinds::OPERAND);
						k->addDebugLocationsFrom(i1);
						k->addDebugLocationsFrom(i2);
						wrappers[i] = k;
						wrappers.erase(wrappers.begin() + i - 2);
						wrappers.erase(wrappers.begin() + i - 2);
						i = i - 2;
					}
				}
			}
		}
	}

	return OK;
}

void SlateContext::removeExpresion(size_t index) {
	delete expresions[index];
	expresions.erase(expresions.begin() + index);
}

void SlateContext::brk() {
	int a = 5;
}
