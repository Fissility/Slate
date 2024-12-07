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
	return defaultDefinitions.objectExists(name) || definitions.objectExists(name);
}

Object* SlateContext::getObject(std::string& name) {
	if (defaultDefinitions.objectExists(name)) return defaultDefinitions.getObject(name);
	return definitions.getObject(name);
}

std::string SlateContext::getObjectName(Object* o) {
	if (defaultDefinitions.objectHasString(o)) {
		return defaultDefinitions.getString(o);
	}
	if (definitions.objectHasString(o)) {
		return definitions.getString(o);
	}
	switch (o->type) {
		case Types::NUMBER: {
			return std::to_string(((Number*)o)->value);
		}
		case Types::TUPLE: {
			Tuple* t = (Tuple*)o;
			std::string out = "(";
			for (size_t i = 0; i < t->length; i++) {
				out += getObjectName((*t)[i]);
				if (i != t->length - 1) out += ",";
			}
			out += ")";
			return out;
		}
		default: {
			return "<" + Types::getTypeName(o->type) + ">";
		}
	}
}


ExpressionInfo SlateContext::newExpression() {
	expresions.push_back(new std::string(""));
	return ExpressionInfo( *(expresions[expresions.size() - 1]), expresions.size() - 1 );
}

std::vector<Object*> SlateContext::processSyntax() {
	// TODO: clean memory
	definitions.clear();
	std::vector<Object*> results;
	for (std::string* s : expresions) {
		results.push_back(processSyntaxLine(*s));
	}
	return results;
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
					if (isEnd(line, localI)) {
						if (scopeIndex != 0) throw CompileControlSequenceFunctionUnclosedBracket(localBegin, localEnd);
						else throw CompileControlSequenceFunctionNotEnoughArguments(localBegin, localEnd);
					}
					std::string next = peek(line, localI);
					localBegin = jump(line, localI);
					localEnd = localI;
					if (next == "{") {
						scopeIndex++;
					}
					else if (scopeIndex == 0) {
						throw CompileControlSequenceFunctionNotEnoughArguments(localBegin, localEnd);
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

bool isConstant(ObjectSyntaxWrapper* wrapper) {
	if (wrapper->type != SyntaxWrapperTypes::KNOWN) return false;
	return((Known*)wrapper)->kind == KnownKinds::OPERAND;
}

bool isUnknown(ObjectSyntaxWrapper* wrapper) {
	return wrapper->type == SyntaxWrapperTypes::UNKNOWN;
}

bool isFunction(ObjectSyntaxWrapper* wrapper) {
	if (wrapper->type != SyntaxWrapperTypes::KNOWN) return false;
	return((Known*)wrapper)->kind == KnownKinds::OPERATOR;
}

bool isComma(ObjectSyntaxWrapper* wrapper) {
	if (wrapper->type != SyntaxWrapperTypes::MARKER) return false;
	return((Marker*)wrapper)->mType == MarkerTypes::COMMA;
}

bool isEquals(ObjectSyntaxWrapper* wrapper) {
	if (wrapper->type != SyntaxWrapperTypes::MARKER) return false;
	return((Marker*)wrapper)->mType == MarkerTypes::EQUALS;
}

bool isBinaryOperator(ObjectSyntaxWrapper* wrapper) {
	if (wrapper->type != SyntaxWrapperTypes::KNOWN) return false;
	return((Known*)wrapper)->kind == KnownKinds::BINARY_OPERATOR;
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
			else if (nameExists(name)) {

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

void SlateContext::printNode(Node* n, size_t spaces) {
	for (size_t i = 0; i < spaces; i++) std::cout << "  ";
	switch (n->type) {
	case NodeTypes::F: {
		std::cout << "F Node " << "(" + getObjectName(((FNode*)n)->function) + ")\n";
		break;
	}
	case NodeTypes::J: {
		std::cout << "J Node\n";
		break;
	}
	case NodeTypes::C: {
		std::cout << "C Node " << "(" + getObjectName(((CNode*)n)->constant) + ")\n";
		break;
	}
	case NodeTypes::U: {
		std::cout << "U Node " << "(\"" + ((UNode*)n)->name + "\")\n";
		break;
	}
	default:
		break;
	}
	for (Node* t : n->tail) printNode(t, spaces + 1);
}

// TODO: account that some control sequences don't have standard paramters \frac, \sqrt etc
std::string SlateContext::generateExpressionString(Node* head) {
	if (head->type == NodeTypes::C) {
		Object* constant = ((CNode*)head)->constant;
		return getObjectName(constant);
	}
	if (head->type == NodeTypes::U) {
		return ((UNode*)head)->name;
	}
	std::vector<std::string> tailStrings;
	for (Node* t : head->tail) tailStrings.push_back(generateExpressionString(t));
	if (head->type == NodeTypes::F) {
		Function* function = ((FNode*)head)->function;
		if (function->type == Types::BINARY_OPERATOR) {
			return tailStrings[0]; // This case is already processed in the required J Node before it
		}
		else {
			return "\\left(" + getObjectName(function) + tailStrings[0] + "\\right)";
		}
	}
	if (head->type == NodeTypes::J) {
		if (!head->head.empty() && head->head[0]->type == NodeTypes::F && ((FNode*)head->head[0])->function->type == Types::BINARY_OPERATOR) {
			return "\\left(" + tailStrings[0] + " " + getObjectName(((FNode*)head->head[0])->function) + " " + tailStrings[1] + "\\right)";
		}
		else {
			std::string out = "\\left(";
			for (size_t i = 0; i < tailStrings.size(); i++) {
				out += tailStrings[i] + " ";
				if (i != tailStrings.size() - 1) out += ",";
			}
			out += "\\right)";
			return out;
		}
	}
	return "Unable to generate string? This should not happen!";
}

std::function<Object* (Tuple*)> generateExpressionImpl(Node* head, std::vector<std::string>& unknowns) {
	if (head->type == NodeTypes::C) {
		Object* constant = ((CNode*)head)->constant;
		return [=](Tuple* t) {
			return constant;
		};
	}
	if (head->type == NodeTypes::U) {
		std::string unknownName = ((UNode*)head)->name;
		size_t getFrom = unknowns.size();
		for (size_t i = 0; i < unknowns.size(); i++) {
			if (unknowns[i] == unknownName) {
				getFrom == i;
				break;
			}
		}
		if (getFrom == unknowns.size()) unknowns.push_back(unknownName);
		return [=](Tuple* t) {
			return (*t)[getFrom];
		};
	}
	std::vector<std::function<Object* (Tuple*)>> tailImpls;
	for (Node* t : head->tail) tailImpls.push_back(generateExpressionImpl(t, unknowns));
	if (head->type == NodeTypes::F) {
		Function* function = ((FNode*)head)->function;
		std::function<Object* (Tuple*)> input = tailImpls[0];
		return [=](Tuple* t) {
			return function->evaluate(input(t));
		};
	}
	if (head->type == NodeTypes::J) {
		size_t inputSize = head->tail.size();
		Object** os = new Object * [inputSize];
		Tuple* joined = new Tuple(inputSize, os);
		return [=](Tuple* t) {
			for (size_t i = 0; i < inputSize; i++) (*joined)[i] = tailImpls[i](t);
			return joined;
		};
	}
}

Expression* SlateContext::generateExprssion(Node* head, std::vector<std::string>& unknowns) {
	std::function<Object* (Tuple*)> impl = generateExpressionImpl(head, unknowns);
	Expression* exp = new Expression(unknowns.size(),impl);
	std::string name = generateExpressionString(head);
	definitions.registerString(exp, name);
	return exp;
}

void pushOperatorToOutput(std::vector<Node*>& output, ObjectSyntaxWrapper* op) {
	if (isFunction(op)) {
		FNode* fnode = new FNode((Function*)((Known*)op)->o);
		fnode->tail.push_back(output.back());
		output.back()->head.push_back(fnode);
		output.pop_back();
		output.push_back(fnode);
	}
	if (isBinaryOperator(op)) {
		if (output.size() < 2) throw CompileFloatingOperator(op->location.begin, op->location.end);
		FNode* fnode = new FNode((Function*)((Known*)op)->o);
		JNode* jnode = new JNode(1);
		Node* right = output.back(); output.pop_back();
		Node* left = output.back(); output.pop_back();
		jnode->tail.push_back(left);
		left->head.push_back(jnode);

		jnode->tail.push_back(right);
		right->head.push_back(jnode);

		fnode->tail.push_back(jnode);
		jnode->head.push_back(fnode);

		output.push_back(fnode);
	}
	if (isComma(op)) {
		if (output.size() < 2) throw CompileFloatingOperator(op->location.begin, op->location.end);
		size_t nestingLevel = op->nestingLevel;
		Node* right = output.back(); output.pop_back();
		Node* left = output.back(); output.pop_back();
		if (left->type == NodeTypes::J && ((JNode*)left)->nestingLevel == nestingLevel) {
			JNode* leftJ = (JNode*)left;
			leftJ->tail.push_back(right);
			right->head.push_back(leftJ);
			output.push_back(leftJ);
		}
		else {
			JNode* jnode = new JNode(nestingLevel);

			jnode->tail.push_back(left);
			left->head.push_back(jnode);
			jnode->tail.push_back(right);
			right->head.push_back(jnode);

			output.push_back(jnode);

		}
	}
}

Expression* SlateContext::parser(std::vector<ObjectSyntaxWrapper*>& wrappers) {
	std::vector<ObjectSyntaxWrapper*> operators;
	std::vector<Node*> output;

	for (ObjectSyntaxWrapper* object : wrappers) {

		if (isConstant(object)) output.push_back(new CNode(((Known*)object)->o));
		else if (isUnknown(object)) output.push_back(new UNode(((Unknown*)object)->name));
		else if (isOperator(object)) {
			while (
				!empty(operators) &&
				isOperator(operators.back()) &&
				precedence(object) <= precedence(operators.back())
				) {

				pushOperatorToOutput(output,operators.back());
				operators.pop_back();
			}

			operators.push_back(object);
		}
		else if (isOpenBracket(object)) operators.push_back(object);
		else if (isClosedBracket(object)) {
			while (!empty(operators) && !isOpenBracket(operators.back())) {
				pushOperatorToOutput(output, operators.back());
				operators.pop_back();
			}
			if (!empty(operators) && isOpenBracket(operators.back())) {
				operators.pop_back();
			}
		}
	}
	while (!empty(operators)) {
		pushOperatorToOutput(output, operators.back());
		operators.pop_back();
	}
	if (!output.empty()) {

		printNode(output[0], 0);

		std::vector<std::string> unknowns;
		return generateExprssion(output[0], unknowns);
	}
	else return nullptr;
}

Object* SlateContext::processSyntaxLine(std::string& line) {

	std::vector<Token> tokens;
	lexer(line, tokens);

	std::vector<ObjectSyntaxWrapper*> wrappers;
	linkTokensToObjects(line, tokens, wrappers);

	Expression* exp = parser(wrappers);

	if (exp != nullptr) {
		if (exp->numberOfVariables == 0) return exp->evalFunc(nullptr);
		return exp;
	}

	return nullptr;

}

void SlateContext::removeExpresion(size_t index) {
	delete expresions[index];
	expresions.erase(expresions.begin() + index);
}

void SlateContext::brk() {
	int a = 5;
}