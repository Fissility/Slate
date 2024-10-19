#include "SlateContext.h"
#include "ParseState.h"
#include "SlateDefinitions.h"
#include <format>
#include "SlateErrors.h"
#include <iostream>
#include <stack>

// ======================================================

/*
* @param c Input charracter
* @return Retruns TRUE if c is a char between a and z or A and Z
*/
bool isAtoB(char c) {
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

bool isSpecialCharacters(std::string s) {
	return std::find(SlateDefinitions::specialCharacters.begin(), SlateDefinitions::specialCharacters.end(), s) != SlateDefinitions::specialCharacters.end();
}
/*
* @param s The string in which it should be searched
* @param i The current position index inside the string
* @return Returns the control sequence present at a specific position in the string, if none is there it returns an empty string
*/
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

/*
* @brief Advances the string index up to the next character that is not whitespace
* @param s The string in which it should skipped
* @param i The current index position in the string
*/
void skipWhiteSpaces(std::string& s, size_t& i) {
	while (s[i] == ' ' && i < s.size()) i++;
}

/*
* @brief Subtracts the string index down to the next character that is not whitespace
* @param s The string in which it should skipped
* @param i The current index position in the string
*/
void skipBackWhiteSpaces(std::string& s, size_t& i) {
	if (i > 0 && s[i - 1] == ' ') {
		while (i > 0 && s[i] == ' ') i--;
	}
}

/*
* @return Returns TRUE if the TeX symbol can be the base of a character
*/
bool isSymbolBase(std::string s) {
	return std::find(SlateDefinitions::symbolBases.begin(), SlateDefinitions::symbolBases.end(), s) != SlateDefinitions::symbolBases.end();
}

/*
* @return Returns TRUE if the TeX symbol can be the flare of a character
*/
bool isSymbolFlare(std::string s) {
	return std::find(SlateDefinitions::symbolFlares.begin(), SlateDefinitions::symbolFlares.end(), s) != SlateDefinitions::symbolFlares.end();
}

/*
* @return Returns TRUE if the TeX symbol is possible binary operator
*/
bool isBinaryOperator(std::string s) {
	return std::find(SlateDefinitions::binaryOperators.begin(), SlateDefinitions::binaryOperators.end(), s) != SlateDefinitions::binaryOperators.end();
}

/*
* @param s The string in which the next term should be searched in
* @param index The index position inside the string
* @return Returns and advances to the next syntactically indivisble structure in the string. Either a character or a control sequence. 
*/
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

/*
* @param s The string in which the next term should be searched in
* @param index The index position inside the string
* @return Returns and does NOT advance to the next syntactically indivisble structure in the string. Either a character or a control sequence.
*/
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
/*
* @brief Advances the index from the start to the end of a subscript
* @param line The string which contains the subscript
* @param i The current position index inside the string
* @param begin The position at which the original syntactical object has started to be lexed
*/
void processSubscript(std::string& line, size_t& i, size_t begin) {
	// If the end is reached then the subscript is empty
	if (isEnd(line, i)) throw CompileEmptySubscript(begin, i);
	// Check if the subscript is done using enclosing brackets
	// In which case going to the end is easy as it just needs to iterate
	// up until the closing bracket is reached
	if (peek(line, i) == "{") {
		begin = jump(line, i);
		if (isEnd(line, i)) throw CompileUnclosedSubscript(begin, i);
		if (peek(line, i) == "}") throw CompileEmptySubscript(begin, i);
		if (!iterateOverBrackets(line, i)) throw CompileUnclosedSubscript(begin, i);
	}
	// Check if the subscript is done using [...]_\left([...]\right)
	// Does not also check if there is a bracket after it as that would not be valid TeX
	else if (peek(line, i) == "\\left") {
		begin = jump(line, i);
		bool passedRight = false;
		bool finished = false;
		while (!finished) {
			if (isEnd(line, i)) throw CompileUnclosedSubscript(begin, i);
			if (passedRight) {
				if (peek(line, i) == ")") finished = true;
				else throw CompileOutOfPlace(begin, i);
			}
			if (peek(line, i) == "\\right") passedRight = true;
			begin = jump(line, i);
		}
	}
	// The remaining option is that the object at the subscripit is directly the object
	else {
		if (isSymbolFlare(peek(line, i))) {
			begin = jump(line, i);
			if (isEnd(line, i)) throw CompileEmptyFlare(begin, i);
			if (peek(line, i) == "{") {
				begin = jump(line, i);
				if (isEnd(line, i)) throw CompileUnclosedFlare(begin, i);
				if (peek(line, i) == "}") throw CompileEmptyFlare(begin, i);
				if (!iterateOverBrackets(line, i)) throw CompileUnclosedFlare(begin, i);
			}
			else {
				if (isEnd(line, i)) throw CompileEmptyFlare(begin, i);
				begin = jump(line, i);
			}
		}
		else {
			if (isEnd(line, i)) throw CompileEmptySubscript(begin, i);
			jump(line, i);
		}
	}
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

void SlateContext::processSyntaxLine(std::string& line) {

	std::vector<Token> tokens;
	lexer(line, tokens);

	std::vector<ObjectSyntaxWrapper*> wrappers;
	linkTokensToObjects(line, tokens, wrappers);

	std::vector<ObjectSyntaxWrapper*> syOut;
	shuntingYard(wrappers, syOut);

}

void SlateContext::lexer(std::string& line, std::vector<Token>& tokens) {
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
				if (isEnd(line, i)) throw CompileUnclosedFraction(begin,end);
				if (peek(line, i) == "{") {
					jump(line, i);
					end = i;
					tokens.push_back(Token(TokenTypes::FRACTION_BEGIN_SECOND, begin, end));
					flags[LexerFlags::FRACTION_OPEN_TOP] = false;
					flags[LexerFlags::FRACTION_OPEN_BOTTOM] = true;
				}
				else throw CompileOutOfPlace(begin, end);
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
				processSubscript(line, i, begin);
				end = i;
			}
			else end = i;

			tokens.push_back(Token(TokenTypes::SYMBOL, begin, end));
			continue;
		}

		// CHECK FOR FLARE START
		if (isSymbolFlare(current)) {
			begin = jump(line, i);
			if (isEnd(line, i)) throw CompileEmptyFlare(begin, end);
			if (peek(line, i) == "{") {
				jump(line, i);
				end = i;
				if (isEnd(line, i)) throw CompileUnclosedFlare(begin, end);
				if (peek(line, i) == "}") throw CompileEmptyFlare(begin, end);
				if (!iterateOverBrackets(line, i)) throw CompileUnclosedFlare(begin, end);
				end = i;
			}
			else {
				if (isEnd(line, i)) throw CompileEmptyFlare(begin, end);
				jump(line, i);
				end = i;
			}
			if (peek(line, i) == "_") {
				jump(line, i);
				end = i;
				processSubscript(line, i, begin);
				end = i;
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
			if (isEnd(line, i)) throw CompileUnclosedFraction(begin, end);
			if (peek(line, i) == "{") {
				jump(line, i);
				end = i;
				tokens.push_back(Token(TokenTypes::FRACTION_BEGIN_FIRST, begin, end));
				flags[LexerFlags::FRACTION_OPEN_TOP] = true;
			}
			else throw CompileUnclosedFraction(begin, end);
			continue;
		}

		throw CompileOutOfPlace(begin, end);
	}
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
void SlateContext::linkTokensToObjects(std::string line,std::vector<Token>& tokens, std::vector<ObjectSyntaxWrapper*>& objects) {
	for (size_t i = 0; i < tokens.size();i++) {
		Token& token = tokens[i];
		size_t nestingLevel = 0;
		for (size_t j = i - 1; j > -1; j--) {
			if (tokens[j].type == TokenTypes::BEGIN_SCOPE) nestingLevel++;
			//                                             V This is needed rn, TODO: move bracket detectin upper in the chain
			if (tokens[j].type == TokenTypes::END_SCOPE && nestingLevel != 0) nestingLevel--;
		}
		std::string name = normaliseName(line.substr(token.location.begin, token.location.end-token.location.begin));
		switch (token.type) {
			case TokenTypes::SYMBOL: {
				if (nameExists(name)) {
					Object* o = nameMap[name];

					objects.push_back(new Known(nameMap[name], KnownKinds::OPERAND, token.location,nestingLevel));
				}
				else {
					objects.push_back(new Unknown(name, token.location,nestingLevel));
				}
				break;
			}
			case TokenTypes::OPERATOR: {
				if (name == "=") {
					objects.push_back(new Marker(MarkerTypes::EQUALS, &token,nestingLevel));

				}
				else if (name == ":") {
					objects.push_back(new Marker(MarkerTypes::COLON, &token,nestingLevel));
					break;
				}
				else if (nameExists(name)) {

					KnownKind kind;

					if (objects.empty()) kind = KnownKinds::OPERAND; // Means it is at the start, then it should be an operand
					else if (isClosedBracket(objects.back()) || !isOperand(objects.back())) kind = KnownKinds::OPERAND; // Means it is after an operator which means it is an operand
					else kind = KnownKinds::BINARY_OPERATOR; // It means it is an operator


					objects.push_back(new Known(nameMap[name], kind, token.location,nestingLevel));
				}
				else throw CompileOperatorNotDefinied(token.location.begin, token.location.end);
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

				objects.push_back(new Marker(MarkerTypes::BEGIN_SCOPE, &token,nestingLevel));
				break;
			}
			case TokenTypes::END_SCOPE: {
				objects.push_back(new Marker(MarkerTypes::END_SCOPE, &token, nestingLevel));
				break;
			}

		}
	}
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
}

// TODO: The whole next section doesn't have the appropriate memory ownership setting, so its riddled with memory leaks, so maybe do that in the future

Tuple* join(Object* o1, Object* o2, size_t nestingLevel1, size_t nestingLevel2, size_t currentLevel) {
	bool firstTuple = o1->type == Types::TUPLE && nestingLevel1 >= currentLevel;
	bool secondTuple = o2->type == Types::TUPLE && nestingLevel2 >= currentLevel;
	size_t size = 0;
	if (firstTuple) size += ((Tuple*)o1)->length;
	else size++;
	if (secondTuple) size += ((Tuple*)o2)->length;
	else size++;
	Object** arr = new Object * [size];
	Tuple* t = new Tuple(size, arr);
	size_t index = 0;
	if (firstTuple) {
		Tuple* tOther = (Tuple*)o1;
		for (size_t i = 0; i < tOther->length; i++) {
			arr[index++] = (*tOther)[i];
		}
	}
	else arr[index++] = o1;
	if (secondTuple) {
		Tuple* tOther = (Tuple*)o1;
		for (size_t i = 0; i < tOther->length; i++) {
			arr[index++] = (*tOther)[i];
		}
	}
}

Known* join_kk(Known* k1, Known* k2, size_t commaLevel) {
	return new Known(
		join(k1->o, k2->o, k1->nestingLevel, k2->nestingLevel, commaLevel),
		KnownKinds::OPERAND,
		{k1->location.begin,k2->location.end},
		commaLevel
	);
}

Dependent* join_uu(Unknown* u1, Unknown* u2, size_t commaLevel) {
	size_t nestingLevelU1 = u1->nestingLevel;
	size_t nestingLevelU2 = u2->nestingLevel;
	Expression* exp = new Expression(1, [=](Object* o[]) {
		return join(o[0], o[1], nestingLevelU1, nestingLevelU2, commaLevel);
	});
	exp->setInverse([=](Object* o,Object* r[]) {
		r[0] = (*(Tuple*)o)[0];
		r[1] = (*(Tuple*)o)[1];
	});
	Dependent* dep = new Dependent(exp, { u1->location.begin,u2->location.end }, commaLevel);
	dep->addDependecy(u1);
	dep->addDependecy(u2);
	return dep;
}

Dependent* join_dd(Dependent* d1, Dependent* d2, size_t commaLevel) {
	size_t nestingLevelD1 = d1->nestingLevel;
	size_t nestingLevelD2 = d2->nestingLevel;

	size_t unknownsCountD1 = d1->uknownsCount();
	size_t unknownsCountD2 = d2->uknownsCount();

	StringLocation locationD1 = d1->location;
	StringLocation locationD2 = d2->location;

	Expression* expD1 = d1->exp;
	Expression* expD2 = d2->exp;
	Expression* exp = new Expression(unknownsCountD1 + unknownsCountD2, [=](Object* o[]) {
		return join(expD1->evalFunc(&o[0]), expD2->evalFunc(&o[unknownsCountD1]), nestingLevelD1, nestingLevelD2, commaLevel);
	});
	exp->setInverse([=](Object* o, Object* r[]) {
		Tuple* t = (Tuple*)o;
		if (!expD1->hasInverse) throw RuntimeNoInverse(locationD1.begin,locationD1.end);
		if (!expD2->hasInverse) throw RuntimeNoInverse(locationD1.begin,locationD2.end);
		// TODO: pretty sure this not correct in general so fix this later, will work for most cases
		Tuple t1(t->length - 1, &(*t)[0]);
		Tuple t2(1, &(*t)[t->length-1]);
		expD1->reverseFunc(&t1, &r[0]);
		expD2->reverseFunc(&t2, &r[unknownsCountD1]);
	});
	Dependent* dep = new Dependent(exp, { d1->location.begin,d2->location.end }, commaLevel);
	dep->addAllDependeciesFrom(d1);
	dep->addAllDependeciesFrom(d2);
	return dep;

}

Dependent* join_uk(Unknown* u, Known* k, size_t commaLevel) {
	size_t nestingLevelU = u->nestingLevel;
	size_t nestingLevelK = k->nestingLevel;
	Object* ko = k->o;
	Expression* exp = new Expression(1, [=](Object* o[]) {
		return join(o[0], ko, nestingLevelU, nestingLevelK, commaLevel);
	});
	exp->setInverse([=](Object* o, Object* r[]) {
		r[0] = (*(Tuple*)o)[0];
	});
	Dependent* dep = new Dependent(exp, { u->location.begin,k->location.end }, commaLevel);
	dep->addDependecy(u);
	return dep;
}

Dependent* join_ku(Known* k, Unknown* u, size_t commaLevel) {
	size_t nestingLevelU = u->nestingLevel;
	size_t nestingLevelK = k->nestingLevel;
	Object* ko = k->o;
	Expression* exp = new Expression(1, [=](Object* o[]) {
		return join(ko, o[0], nestingLevelU, nestingLevelK, commaLevel);
	});
	exp->setInverse([=](Object* o, Object* r[]) {
		r[0] = (*(Tuple*)o)[1];
	});
	Dependent* dep = new Dependent(exp, { k->location.begin,u->location.end }, commaLevel);
	dep->addDependecy(u);
	return dep;
}

Dependent* join_ud(Unknown* u, Dependent* d, size_t commaLevel) {
	size_t nestingLevelU = u->nestingLevel;

	size_t nestingLevelD = d->nestingLevel;

	StringLocation locationD = d->location;
	Expression* expD = d->exp;
	Expression* exp = new Expression(1 + d->uknownsCount(), [=](Object* o[]) {
		return join(o[0], expD->evalFunc(&o[1]), nestingLevelU, nestingLevelD, commaLevel);
	});

	exp->setInverse([=](Object* o, Object* r[]) {
		Tuple* t = (Tuple*)o;
		if (!expD->hasInverse) throw RuntimeNoInverse(locationD.begin,locationD.end);
		Tuple t1(t->length - 1, &(*t)[1]);
		r[0] = (*t)[0];
		expD->reverseFunc(&t1, &r[1]);
	});
	Dependent* dep = new Dependent(exp, { u->location.begin,d->location.end }, commaLevel);
	dep->addDependecy(u);
	dep->addAllDependeciesFrom(d);
	return dep;
}

Dependent* join_du(Dependent* d, Unknown* u, size_t commaLevel) {
	size_t nestingLevelU = u->nestingLevel;
	size_t nestingLevelD = d->nestingLevel;
	size_t dUnknownsCount = d->uknownsCount();
	StringLocation locationD = d->location;
	Expression* expD = d->exp;
	Expression* exp = new Expression(1 + d->uknownsCount(), [=](Object* o[]) {
		return join(expD->evalFunc(&o[0]),o[dUnknownsCount], nestingLevelU, nestingLevelD, commaLevel);
	});
	exp->setInverse([=](Object* o, Object* r[]) {
		Tuple* t = (Tuple*)o;
		if (!expD->hasInverse) throw RuntimeNoInverse(locationD.begin,locationD.end);
		Tuple t1(t->length - 1, &(*t)[0]);
		r[dUnknownsCount] = (*t)[t->length-1];
		expD->reverseFunc(&t1, &r[0]);
	});
	Dependent* dep = new Dependent(exp, { u->location.begin,d->location.end }, commaLevel);
	dep->addAllDependeciesFrom(d);
	dep->addDependecy(u);
	return dep;
}

Dependent* join_kd(Known* k, Dependent* d, size_t commaLevel) {
	size_t nestingLevelK = k->nestingLevel;
	size_t nestingLevelD = d->nestingLevel;
	size_t dUnknownsCount = d->uknownsCount();
	Object* ko = k->o;
	StringLocation locationD = d->location;
	Expression* expD = d->exp;
	Expression* exp = new Expression(d->uknownsCount(), [=](Object* o[]) {
		return join(ko, expD->evalFunc(o), nestingLevelK, nestingLevelD, commaLevel);
	});
	exp->setInverse([=](Object* o, Object* r[]) {
		Tuple* t = (Tuple*)o;
		if (!expD->hasInverse) throw RuntimeNoInverse(locationD.begin,locationD.end);
		Tuple t1(t->length - 1, &(*t)[1]);
		expD->reverseFunc(&t1, &r[0]);
	});
	Dependent* dep = new Dependent(exp, { k->location.begin,d->location.end }, commaLevel);
	dep->addAllDependeciesFrom(d);
	return dep;
}

Dependent* join_dk(Dependent* d, Known* k, size_t commaLevel) {
	size_t nestingLevelK = k->nestingLevel;
	size_t nestingLevelD = d->nestingLevel;
	size_t dUnknownsCount = d->uknownsCount();
	Object* ko = k->o;
	StringLocation locationD = d->location;
	Expression* expD = d->exp;
	Expression* exp = new Expression(d->uknownsCount(), [=](Object* o[]) {
		return join(expD->evalFunc(o), ko, nestingLevelK, nestingLevelD, commaLevel);
	});
	exp->setInverse([=](Object* o, Object* r[]) {
		Tuple* t = (Tuple*)o;
		if (!expD->hasInverse) throw RuntimeNoInverse(locationD.begin,locationD.end);
		// TODO: pretty sure this not correct in general so fix this later, will work for most cases
		Tuple t1(t->length - 1, &(*t)[0]);
		expD->reverseFunc(&t1, &r[0]);
	});
	Dependent* dep = new Dependent(exp, { d->location.begin,k->location.end }, commaLevel);
	dep->addAllDependeciesFrom(d);
	return dep;
}

ObjectSyntaxWrapper* joinObjects(ObjectSyntaxWrapper* left, ObjectSyntaxWrapper* right, size_t commaLevel = SIZE_MAX) {
	if (left->type == SyntaxWrapperTypes::KNOWN) {
		if (right->type == SyntaxWrapperTypes::KNOWN) return join_kk((Known*)left, (Known*)right, commaLevel);
		if (right->type == SyntaxWrapperTypes::UNKNOWN) return join_ku((Known*)left, (Unknown*)right, commaLevel);
		if (right->type == SyntaxWrapperTypes::DEPENDENT) return join_kd((Known*)left, (Dependent*)right, commaLevel);
	}
	if (left->type == SyntaxWrapperTypes::UNKNOWN) {
		if (right->type == SyntaxWrapperTypes::KNOWN) return join_uk((Unknown*)left, (Known*)right, commaLevel);
		if (right->type == SyntaxWrapperTypes::UNKNOWN) return join_uu((Unknown*)left, (Unknown*)right, commaLevel);
		if (right->type == SyntaxWrapperTypes::DEPENDENT) return join_ud((Unknown*)left, (Dependent*)right, commaLevel);
	}
	if (left->type == SyntaxWrapperTypes::DEPENDENT) {
		if (right->type == SyntaxWrapperTypes::KNOWN) return join_dk((Dependent*)left, (Known*)right, commaLevel);
		if (right->type == SyntaxWrapperTypes::UNKNOWN) return join_du((Dependent*)left, (Unknown*)right, commaLevel);
		if (right->type == SyntaxWrapperTypes::DEPENDENT) return join_dd((Dependent*)left, (Dependent*)right, commaLevel);
	}
	return nullptr;
}

Known* func_k(Known* func, Known* k, bool isBinary) {
	Function* f = (Function*)func;
	if (!f->domain->in(k->o)) throw CompileDomainException(func->location.begin, func->location.end);
	Object* result = f->evaluate(k->o);

	StringLocation location;

	if (isBinary) location = { k->location.begin,k->location.end };
	else location = { func->location.begin,k->location.end };

	return new Known(result, KnownKinds::OPERAND, location, func->nestingLevel);
}

Dependent* func_u(Known* func, Unknown* u) {
	Function* f = (Function*)func;

	StringLocation functionLocation = func->location;

	Expression* exp = new Expression(1, [=](Object* o[]) {
		if (!f->domain->in(o[0])) throw RuntimeDomainException(functionLocation.begin, functionLocation.end);
		return f->evaluate(o[0]);
	});

	Dependent* dep = new Dependent(exp, { func->location.begin,u->location.end }, func->nestingLevel);
	dep->addDependecy(u);
	return dep;

}

Dependent* func_d(Known* func, Dependent* d,bool isBinary) {
	Function* f = (Function*)func;

	StringLocation functionLocation = func->location;

	Expression* expD = d->exp;

	Expression* exp = new Expression(d->uknownsCount(), [=](Object* o[]) {
		Object* result = expD->evalFunc(o);
		if (!f->domain->in(result)) throw RuntimeDomainException(functionLocation.begin, functionLocation.end);
		return f->evaluate(result);
	});

	StringLocation location;
	if (isBinary) location = { d->location.begin,d->location.end };
	else location = { func->location.begin,d->location.end };

	Dependent* dep = new Dependent(exp, location, func->nestingLevel);
	dep->addAllDependeciesFrom(d);
	return dep;

}

// TODO: A way to mark defition headers


void SlateContext::parser(std::vector<ObjectSyntaxWrapper*>& wrappers) {
	// TODO: function composition function

	bool done = false;

	while (!done) {
		for (size_t i = 0; i < wrappers.size(); i++) {

			ObjectSyntaxWrapper* ow = wrappers[i];
			if (ow->type == SyntaxWrapperTypes::KNOWN) {
				Known* iow = (Known*)ow;

				if (iow->kind == KnownKinds::BINARY_OPERATOR) {
					if (wrappers.size() < 2) throw CompileFloatingOperator(ow->assosciatedToken->location.begin, ow->assosciatedToken->location.end);

					SyntaxWrapperType t1 = wrappers[i - 1]->type;
					SyntaxWrapperType t2 = wrappers[i - 2]->type;



				}
			}
		}
	}
}

void SlateContext::removeExpresion(size_t index) {
	delete expresions[index];
	expresions.erase(expresions.begin() + index);
}

void SlateContext::brk() {
	int a = 5;
}