#include "SlateContext.h"
#include "ParseState.h"
#include "SlateDefinitions.h"
#include <format>
#include "SlateErrors.h"
#include "objects/tuple/BiCategory.h" //X
#include <iostream>
#include <stack>

// ======================================================

/*
* @param c Input charracter
* @return Retruns TRUE if c is a char between a and z or A and Z
*/
bool isAtoZ(char c) {
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

/*
* @param c Input charracter
* @return Retruns TRUE if c is a char between 0 and 9
*/
bool is0to9(char c) {
	return '0' <= c && c <= '9';
}

bool isControlSeqeuenceCharacter(std::string s) {
	return std::find(SlateDefinitions::controlSeqeuenceCharacters.begin(), SlateDefinitions::controlSeqeuenceCharacters.end(), s) != SlateDefinitions::controlSeqeuenceCharacters.end();
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
	if (isControlSeqeuenceCharacter(std::string(1, s[i])) && i < s.size()) {
		name += s[i];
		return name;
	}
	while (true) {
		if ((isAtoZ(s[i]) || is0to9(s[i])) && i < s.size()) name += s[i];
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

bool isControlSequenceFunction(std::string s) {
	return SlateDefinitions::controlSequenceFunctions.count(s);
}

/*
* @param s The string in which the next term should be searched in
* @param index The index position inside the string
* @return Returns and advances to the next syntactically indivisble structure in the string. Either a character or a control sequence. 
*/
size_t jump(std::string& s, size_t& index) {
	skipWhiteSpaces(s, index);
	size_t begin = index;
	if (is0to9(s[index])) {
		index++;
		while (index < s.size() && (is0to9(s[index]) || s[index] == '.')) index++;
	} else if (s[index] != '\\') {
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
	if (is0to9(s[index])) {
		std::string result = "";
		result += s[index++];
		// TODO: option to change the decimal delimitator between . and ,
		while (index < s.size() && (is0to9(s[index]) || s[index] == '.')) {
			index++;
			result += s[index];
		}
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
	// The remaining option is that the object at the subscript is directly the control sequence
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


bool SlateContext::nameExists(std::string& name) {
	return definitions.find(name) != definitions.end() || defaultDefinitions->find(name) != defaultDefinitions->end();
}

Object* SlateContext::getObject(std::string& name) {
	if (definitions.find(name) != definitions.end()) return definitions[name];
	if (defaultDefinitions->find(name) != defaultDefinitions->end()) return (*defaultDefinitions)[name];
	return nullptr;
}

void SlateContext::addDefinition(std::string& name, Object* object) {
	definitions[name] = object;
}

ExpressionInfo SlateContext::newExpression() {
	expresions.push_back(new std::string(""));
	return ExpressionInfo( *(expresions[expresions.size() - 1]), expresions.size() - 1 );
}

std::vector<Object*> SlateContext::processSyntax() {
	// TODO: clean memory
	definitions.clear();
	names.clear();
	std::vector<Object*> results;
	for (std::string* s : expresions) {
		results.push_back(processSyntaxLine(*s));
	}
	return results;
}

bool isOperator(ObjectSyntaxWrapper* wrapper) {
	if (wrapper->type == SyntaxWrapperTypes::MARKER) {
		Marker* marker = ((Marker*)wrapper);
		if (marker->mType == MarkerTypes::EQUALS || marker->mType == MarkerTypes::COLON || marker->mType == MarkerTypes::COMMA) return true;
	}
	if (wrapper->type == SyntaxWrapperTypes::KNOWN) {
		Known* i = (Known*)wrapper;
		return i->kind == KnownKinds::OPERATOR || i->kind == KnownKinds::BINARY_OPERATOR;
	}
	return false;
}

bool isOpenBracket(ObjectSyntaxWrapper* wrapper) {
	return wrapper->type == SyntaxWrapperTypes::MARKER && ((Marker*)wrapper)->mType == MarkerTypes::BEGIN_SCOPE;
}

bool isClosedBracket(ObjectSyntaxWrapper* wrapper) {
	return wrapper->type == SyntaxWrapperTypes::MARKER && ((Marker*)wrapper)->mType == MarkerTypes::END_SCOPE;
}

// TODO, implement isOperand
bool isOperand(ObjectSyntaxWrapper* wrapper) {
	return !isOperator(wrapper) && !isOpenBracket(wrapper) && !isClosedBracket(wrapper);
}

void SlateContext::lexer(std::string& line, std::vector<Token>& tokens) {
	size_t begin = 0;
	size_t end = 0;
	size_t i = 0;

	std::vector<Token> tokenOverrides;

	while (!isEnd(line, i)) {
		std::string current = peek(line, i);

		if (current == "{") {
			begin = jump(line, i);
			end = i;
			pushOverride({ begin,end }, tokens, tokenOverrides);
			continue;
		}

		if (current == "}") {
			begin = jump(line, i);
			end = i;
			pushOverride({ begin,end }, tokens, tokenOverrides);
			continue;
		}

		if (current == ",") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(Token(TokenTypes::SYMBOL, begin, end));
			continue;
		}

		if (is0to9(current[0])) {
			begin = jump(line, i);
			end = i;
			tokens.push_back(Token(TokenTypes::NUMERICAL_CONSTANT, begin, end));
			continue;
		}

		// CHECK FOR SYMBOL START
		if (isSymbolBase(current)) {
			begin = jump(line, i);
			end = i;
			if (isControlSequenceFunction(current)) {
				size_t remainingCtrlSqArguments = std::atoi(SlateDefinitions::controlSequenceFunctions[current].c_str());
				size_t scopeIndex = 0;
				size_t localI = i;
				size_t localBegin = 0;
				size_t localEnd = 0;
				while (remainingCtrlSqArguments != 0) {
					std::string next = peek(line, localI);
					localBegin = jump(line, localI);
					localEnd = localI;
					if (next == "{") {
						scopeIndex++;
					}
					if (next == "}") {
						if (scopeIndex == 0) throw CompileBracketNotOpened(localBegin, localEnd);
						scopeIndex--;
						if (scopeIndex == 0) {
							remainingCtrlSqArguments--;
							if (remainingCtrlSqArguments == 0) {
								tokenOverrides.push_back(Token(TokenTypes::END_SCOPE, localBegin, localEnd));
							}
							else {
								tokenOverrides.push_back(Token(TokenTypes::SYMBOL, localBegin, localEnd));
							}
						}
					}
				}
				tokens.push_back(Token(TokenTypes::SYMBOL, begin, end));
				tokens.push_back(Token(TokenTypes::BEGIN_SCOPE, end, end));
			}
			else {
				if (peek(line, i) == "_") {
					jump(line, i);
					end = i;
					processSubscript(line, i, begin);
					end = i;
				}
				else end = i;
				tokens.push_back(Token(TokenTypes::SYMBOL, begin, end));
			}
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

		throw CompileOutOfPlace(begin, end);
	}
}

/*
* Links names to defined objects and classifies them based on their role in the expression
*/
void SlateContext::linkTokensToObjects(std::string line, std::vector<Token>& tokens, std::vector<ObjectSyntaxWrapper*>& objects) {
	for (size_t i = 0; i < tokens.size(); i++) {
		Token& token = tokens[i];
		size_t nestingLevel = 0;
		for (size_t j = 0; j <= i; j++) {
			if (tokens[j].type == TokenTypes::BEGIN_SCOPE) nestingLevel++;
			//                                             V This is needed rn, TODO: move bracket detectin upper in the chain
			if (tokens[j].type == TokenTypes::END_SCOPE && nestingLevel != 0) nestingLevel--;
		}
		switch (token.type) {
		case TokenTypes::NUMERICAL_CONSTANT: {
			// Numerical constants are directly converted to the number object they represent and packed as a known object
			size_t begin = token.location.begin;
			size_t end = token.location.end;
			double wholeTotal = 0;
			double dotPlace = 1;
			bool pastDot = false;
			for (size_t p = begin; p < end; p++) {
				if (line[p] != '.') {
					wholeTotal = wholeTotal * 10 + (line[p] - '0');
					if (pastDot) dotPlace /= 10;
				}
				else pastDot = true;
			}
			double result = wholeTotal * dotPlace;
			objects.push_back(new Known(new Number(result), KnownKinds::OPERAND, token.location, nestingLevel));
			break;
		}
		case TokenTypes::SYMBOL: {
			// Signifies it is just a name, it can either be some external varaible or a function
			std::string name = line.substr(token.location.begin, token.location.end - token.location.begin);
		
			if (name == "=") {
				objects.push_back(new Marker(MarkerTypes::EQUALS, token.location, nestingLevel));

			}
			else if (name == ":") {
				objects.push_back(new Marker(MarkerTypes::COLON, token.location, nestingLevel));
			}
			else if (name == "," || name == "}") {
				objects.push_back(new Marker(MarkerTypes::COMMA, token.location, nestingLevel));
			}
			else if (nameExists(name = normaliseName(name))) {

				Object* o = getObject(name);
				// If the next token is the begining of a scope and the object linked to the current token is a function
				// Then it means that the evaluation of a function is taking place
				// V V
				// f (...)
				if (i != tokens.size() - 1 && tokens[i + 1].type == TokenTypes::BEGIN_SCOPE && o->type == Types::FUNCTION) {
					objects.push_back(new Known(getObject(name), KnownKinds::OPERATOR, token.location, nestingLevel));
				}
				else if (o->type == Types::BINARY_OPERATOR) {
					KnownKind kind;
					if (objects.empty() || (!isOperand(objects.back()) && !isClosedBracket(objects.back()))) {
						if (((BinaryOperator*)getObject(name))->canBeUnary && i != tokens.size() - 1 && (tokens[i + 1].type == TokenTypes::SYMBOL || tokens[i + 1].type == TokenTypes::NUMERICAL_CONSTANT)) {
							kind = KnownKinds::OPERATOR;
						}
						else {
							kind = KnownKinds::OPERAND;
						}
					}
					else kind = KnownKinds::BINARY_OPERATOR; // It means it is an operator


					objects.push_back(new Known(getObject(name), kind, token.location, nestingLevel));
				}
				// Otherwise if it is a function then it used as an operand (f+g,f-g,f\\circ g) or it isn't a function in which case it is a variable
				else {
					objects.push_back(new Known(getObject(name), KnownKinds::OPERAND, token.location, nestingLevel));
				}
			}
			else {
				Unknown* u = new Unknown(name, token.location, nestingLevel);
				if (i != tokens.size() - 1 && tokens[i + 1].type == TokenTypes::BEGIN_SCOPE) {
					u->canBeFunctionOrExpression = true;
				}
				objects.push_back(new Unknown(name, token.location, nestingLevel));
			}
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

			objects.push_back(new Marker(MarkerTypes::BEGIN_SCOPE, token.location, nestingLevel));
			break;
		}
		case TokenTypes::END_SCOPE: {
			objects.push_back(new Marker(MarkerTypes::END_SCOPE, token.location, nestingLevel));
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
		case MarkerTypes::COMMA:
			return INT_MIN + 2;
		default:
			break;
		}
	}
	if (isOperator(wrapper)) {
		Known* k = (Known*)wrapper;
		if (k->kind == KnownKinds::OPERATOR) return INT_MAX;
		if (k->kind == KnownKinds::BINARY_OPERATOR) {
			return ((BinaryOperator*)((Known*)wrapper)->o)->precedence;
		}
	}
	return 0;
}

void shuntingYard(std::string line,std::vector<ObjectSyntaxWrapper*>& wrappers, std::vector<ObjectSyntaxWrapper*>& output) {

	std::vector<ObjectSyntaxWrapper*> operators;

	for (ObjectSyntaxWrapper* object : wrappers) {

		std::cout << '\n';

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

Object* SlateContext::processSyntaxLine(std::string& line) {

	std::vector<Token> tokens;
	lexer(line, tokens);

	std::vector<ObjectSyntaxWrapper*> wrappers;
	linkTokensToObjects(line, tokens, wrappers);

	std::vector<ObjectSyntaxWrapper*> syOut;
	shuntingYard(line, wrappers, syOut);

	parser(syOut);

	//                       V this should in theory note be needed
	if (syOut.size() != 0 && syOut[0]->type == SyntaxWrapperTypes::KNOWN) return ((Known*)syOut[0])->o;
	return nullptr;

}

void pushOverride(StringLocation location, std::vector<Token>& tokenList, std::vector<Token>& tokenOverrides) {
	for (size_t i = 0; i < tokenOverrides.size(); i++) {
		StringLocation ol = tokenOverrides[i].location;
		if (location == ol) {
			tokenList.push_back(tokenOverrides[i]);
			tokenOverrides.erase(tokenOverrides.begin() + i);
			return;
		}
	}
}

void pushToken(Token t, std::vector<Token>& tokenList, std::vector<Token>& tokenOverrides) {
	StringLocation l = t.location;
	for (size_t i = 0; i < tokenOverrides.size(); i++) {
		StringLocation ol = tokenOverrides[i].location;
		if (l == ol) {
			tokenList.push_back(tokenOverrides[i]);
			tokenOverrides.erase(tokenOverrides.begin() + i);
			return;
		}
	}
	tokenList.push_back(t);
}

// TODO: The whole next section doesn't have the appropriate memory ownership setting, so its riddled with memory leaks, so maybe do that in the future

Tuple* join(Object* o1, Object* o2, size_t nestingLevel1, size_t nestingLevel2, size_t currentLevel) {
	bool firstTuple = o1->type == Types::TUPLE && nestingLevel1 == currentLevel;
	bool secondTuple = o2->type == Types::TUPLE && nestingLevel2 == currentLevel;
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
		Tuple* tOther = (Tuple*)o2;
		for (size_t i = 0; i < tOther->length; i++) {
			arr[index++] = (*tOther)[i];
		}
	}
	else arr[index++] = o2;
	return t;
}

Known* join_kk(Known* k1, Known* k2, size_t commaLevel) {
	// Only one that doesn't perform some function compositions only returns a known value equal to the tuple of the two
	return new Known(
		join(k1->o, k2->o, k1->nestingLevel, k2->nestingLevel, commaLevel),
		KnownKinds::OPERAND,
		{k1->location.begin,k2->location.end},
		commaLevel
	);
}

// Any inverse implementation besides UU may not be correct going forward

Dependent* join_uu(Unknown* u1, Unknown* u2, size_t commaLevel) {
	size_t nestingLevelU1 = u1->nestingLevel;
	size_t nestingLevelU2 = u2->nestingLevel;
	Expression* exp = new Expression(1, 
	[=](Object* o[]) { // Eval
		return join(o[0], o[1], nestingLevelU1, nestingLevelU2, commaLevel);
	},
	[=](Object* o,Object* r[]) { // Inverse
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
	Expression* exp = new Expression(unknownsCountD1 + unknownsCountD2, 
	[=](Object* o[]) { // Eval
		return join(expD1->evalFunc(&o[0]), expD2->evalFunc(&o[unknownsCountD1]), nestingLevelD1, nestingLevelD2, commaLevel);
	},
	[=](Object* o, Object* r[]) { // Inverse
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
	Expression* exp = new Expression(1, 
	[=](Object* o[]) {
		return join(o[0], ko, nestingLevelU, nestingLevelK, commaLevel);
	},
	[=](Object* o, Object* r[]) {
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
	},
	[=](Object* o, Object* r[]) {
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
	Expression* exp = new Expression(1 + d->uknownsCount(), 
	[=](Object* o[]) {
		return join(o[0], expD->evalFunc(&o[1]), nestingLevelU, nestingLevelD, commaLevel);
	},
	[=](Object* o, Object* r[]) {
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
	},
	[=](Object* o, Object* r[]) {
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

	Expression* exp = new Expression(d->uknownsCount(), 
	[=](Object* o[]) { // Eval
		return join(ko, expD->evalFunc(o), nestingLevelK, nestingLevelD, commaLevel);
	}, 
	[=](Object* o, Object* r[]) { // Reverse
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
	},
	[=](Object* o, Object* r[]) {
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
	// This also doesn't nest any functions similar to join_kk
	Function* f = (Function*)func->o;
	if (!f->domain->in(k->o)) throw CompileDomainException(func->location.begin, func->location.end);
	Object* result = f->evaluate(k->o);

	StringLocation location;

	if (isBinary) location = { k->location.begin,k->location.end };
	else location = { func->location.begin,k->location.end };

	return new Known(result, KnownKinds::OPERAND, location, func->nestingLevel);
}

Dependent* func_u(Known* func, Unknown* u) {
	// An unknown is a signle syntaxtical unmodified object thus, it doesnt need to know if it is a binary function for propper syntax feedback
	Function* f = (Function*)func->o;

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
	Function* f = (Function*)func->o;

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

ObjectSyntaxWrapper* funcPass(Known* func, ObjectSyntaxWrapper* obj,bool isBinary) {
	SyntaxWrapperType type = obj->type;
	if (type == SyntaxWrapperTypes::KNOWN) return func_k(func, (Known*)obj, isBinary);
	if (type == SyntaxWrapperTypes::UNKNOWN) return func_u(func, (Unknown*)obj);
	if (type == SyntaxWrapperTypes::DEPENDENT) return func_d(func, (Dependent*)obj, isBinary);
	return nullptr;
}

// TODO: A way to mark defition headers

void removeAt(size_t index, size_t amount, std::vector<ObjectSyntaxWrapper*>& wrappers) {
	for (size_t i = 0; i < amount; i++) {
		wrappers.erase(wrappers.begin() + index);
	}
}

void addAt(size_t index, ObjectSyntaxWrapper* obj, std::vector<ObjectSyntaxWrapper*>& wrappers) {
	wrappers.insert(wrappers.begin() + index, obj);
}

void SlateContext::parser(std::vector<ObjectSyntaxWrapper*>& wrappers) {
	// TODO: function composition function

	bool done = false;

	for (size_t i = 0; i < wrappers.size(); i++) {

		ObjectSyntaxWrapper* ow = wrappers[i];
		if (ow->type == SyntaxWrapperTypes::KNOWN) {
			Known* iow = (Known*)ow;

			if (iow->kind == KnownKinds::OPERATOR) {
				if (wrappers.size() < 1 || i < 1) throw CompileFloatingOperator(ow->location.begin, ow->location.end);

				ObjectSyntaxWrapper* w = wrappers[i - 1];
				ObjectSyntaxWrapper* out = funcPass(iow, w, false);
				i -= 1;
				removeAt(i, 2, wrappers);
				addAt(i, out, wrappers);
			}

			if (iow->kind == KnownKinds::BINARY_OPERATOR) {
				if (wrappers.size() < 3 || i < 2) throw CompileFloatingOperator(ow->location.begin, ow->location.end);

				ObjectSyntaxWrapper* w1 = wrappers[i - 2];
				ObjectSyntaxWrapper* w2 = wrappers[i - 1];
				ObjectSyntaxWrapper* joined = joinObjects(w1, w2);
				ObjectSyntaxWrapper* out = funcPass(iow, joined, true);
				i -= 2;
				removeAt(i, 3, wrappers);
				addAt(i, out, wrappers);
			}
		}
		if (ow->type == SyntaxWrapperTypes::MARKER) {
			Marker* iow = (Marker*)ow;

			if (iow->mType == MarkerTypes::COMMA) {
				if (wrappers.size() < 3 || i < 2) throw CompileFloatingOperator(ow->location.begin, ow->location.end);

				ObjectSyntaxWrapper* w1 = wrappers[i - 2];
				ObjectSyntaxWrapper* w2 = wrappers[i - 1];
				ObjectSyntaxWrapper* joined = joinObjects(w1, w2, iow->nestingLevel);
				i -= 2;
				removeAt(i, 3, wrappers);
				addAt(i, joined, wrappers);

			}

			if (iow->mType == MarkerTypes::EQUALS) {
				if (wrappers.size() == 3) {
					ObjectSyntaxWrapper* w1 = wrappers[i - 2];
					ObjectSyntaxWrapper* w2 = wrappers[i - 1];
					SyntaxWrapperType t1 = w1->type;
					SyntaxWrapperType t2 = w2->type;
					if (t1 == SyntaxWrapperTypes::UNKNOWN && t2 == SyntaxWrapperTypes::KNOWN) {
						addDefinition(((Unknown*)w1)->name, ((Known*)w2)->o);
					}
					else if (t2 == SyntaxWrapperTypes::UNKNOWN && t1 == SyntaxWrapperTypes::KNOWN) {
						addDefinition(((Unknown*)w2)->name, ((Known*)w1)->o);
					}
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