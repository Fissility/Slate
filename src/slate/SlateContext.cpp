#include "SlateContext.h"
#include "ParseState.h"
#include "SlateDefinitions.h"
#include <format>
#include "SlateErrors.h"
#include <iostream>

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

void SlateContext::processSyntax() {
	for (std::string* s : expresions) {
		processSyntaxLine(*s);
	}
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

ParseError processSubscript(std::string& line, size_t& i) {
	if (isEnd(line, i)) return EMPTY_SUBSCRIPT;
	if (peek(line, i) == "{") {
		jump(line, i);
		if (isEnd(line, i)) return UNCLOSED_SUBSCRIPT;
		if (peek(line, i) == "}") return EMPTY_SUBSCRIPT;
		if (!iterateOverBrackets(line, i)) return UNCLOSED_SUBSCRIPT;
	}
	else if (peek(line, i) == "\\left") {
		jump(line, i);
		bool passedRight = false;
		bool finished = false;
		while (!finished) {
			if (isEnd(line, i)) return UNCLOSED_SUBSCRIPT;
			if (passedRight) {
				if (peek(line, i) == ")") finished = true;
				else return OUT_OF_PLACE;
			}
			if (peek(line, i) == "\\right") passedRight = true;
			jump(line, i);
		}
	}
	else {
		if (isSymbolFlare(peek(line, i))) {
			jump(line, i);
			if (isEnd(line, i)) return EMPTY_FLARE;
			if (peek(line, i) == "{") {
				jump(line, i);
				if (isEnd(line, i)) return UNCLOSED_FLARE;
				if (peek(line, i) == "}") return EMPTY_FLARE;
				if (!iterateOverBrackets(line, i)) return UNCLOSED_FLARE;
			}
			else {
				if (isEnd(line, i)) return EMPTY_FLARE;
				jump(line, i);
			}
		}
		else {
			if (isEnd(line, i)) return EMPTY_SUBSCRIPT;
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
}

ExpressionInfo SlateContext::newExpression() {
	expresions.push_back(new std::string(""));
	return ExpressionInfo( *(expresions[expresions.size() - 1]), expresions.size() - 1 );
}

ParseError SlateContext::lexer(std::string& line, std::vector<Token>& tokens) {
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
				if (isEnd(line, i)) return UNCLOSED_FRACTION;
				if (peek(line, i) == "{") {
					jump(line, i);
					end = i;
					tokens.push_back(Token(FRACTION_BEGIN_SECOND, begin, end));
					flags[LexerFlags::FRACTION_OPEN_TOP] = false;
					flags[LexerFlags::FRACTION_OPEN_BOTTOM] = true;
				}
				else return OUT_OF_PLACE;
			}
			else if (flags[LexerFlags::FRACTION_OPEN_BOTTOM]) {
				begin = jump(line, i);
				end = i;
				tokens.push_back(Token(FRACTION_END, begin, end));
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
				ParseError err = processSubscript(line, i);
				end = i;
				if (err.id != 0) return err;
			}
			else end = i;

			tokens.push_back(Token(SYMBOL, begin, end));
			continue;
		}

		// CHECK FOR FLARE START
		if (isSymbolFlare(current)) {
			begin = jump(line, i);
			if (isEnd(line, i)) return EMPTY_FLARE;
			if (peek(line, i) == "{") {
				jump(line, i);
				end = i;
				if (isEnd(line, i)) return UNCLOSED_FLARE;
				if (peek(line, i) == "}") return EMPTY_FLARE;
				if (!iterateOverBrackets(line, i)) return UNCLOSED_FLARE;
				end = i;
			}
			else {
				if (isEnd(line, i)) return EMPTY_FLARE;
				jump(line, i);
				end = i;
			}
			if (peek(line, i) == "_") {
				jump(line, i);
				end = i;
				ParseError err = processSubscript(line, i);
				end = i;
				if (err.id != 0) return err;
			}
			tokens.push_back(Token(SYMBOL, begin, end));
			continue;
		}

		// OPEN BRACKET CHECK
		if (current == "(") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(Token(BEGIN_SCOPE, begin, end));
			continue;
		}


		// CLOSED BRACKET CHECK
		if (current == ")") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(Token(END_SCOPE, begin, end));
			continue;
		}

		if (current == "=") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(Token(EQUALS, begin, end));
			continue;
		}

		if (current == "+") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(Token(ADDITION, begin, end));
			continue;
		}

		if (current == "-") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(Token(SUBTRACTION, begin, end));
			continue;
		}

		if (current == "\\cdot") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(Token(MULTIPILCATION, begin, end));
			continue;
		}

		if (current == "\\div") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(Token(DIVSION, begin, end));
			continue;
		}

		if (current == "^") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(Token(SUPERSCRIPT, begin, end));
			continue;
		}

		if (current == "\\frac") {
			begin = jump(line, i);
			if (isEnd(line, i)) return UNCLOSED_FRACTION;
			if (peek(line, i) == "{") {
				jump(line, i);
				end = i;
				tokens.push_back(Token(FRACTION_BEGIN_FIRST, begin, end));
				flags[LexerFlags::FRACTION_OPEN_TOP] = true;
			}
			else return UNCLOSED_FRACTION;
			continue;
		}

		if (current == ":") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(Token(COLON, begin, end));
			continue;
		}

		if (current == "|") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(Token(VERTICAL_SEPARATOR, begin, end));
			continue;
		}

		if (current == "\\Rightarrow") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(Token(MAPS_TO, begin, end));
			continue;
		}

		if (current == "\\times") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(Token(CROSS, begin, end));
			continue;
		}

		if (current == "\\forall") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(Token(FOR_ALL, begin, end));
			continue;
		}

		if (current == "\\in") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(Token(IN, begin, end));
			continue;
		}

		if (current == ",") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(Token(COMMA, begin, end));
			continue;
		}

		return OUT_OF_PLACE;
	}
	return OK;
}

ParseError SlateContext::parser(std::vector<Token>& tokens) {

	// TODO: Implement shunting yard
	// TODO: function composition function

	return OK;
}

// TODO, implement precedence function
int precedence(Token token)
{
	return 1;
}
// TODO, implement isOperator
bool isOperator(Token token)
{
	return true;
}
// TODO, implement rightAssociative
bool rightAssociative(Token token)
{
	return true;
}
// TODO, implement isOperand
bool isOperand(Token token)
{
	return true;
}
// TODO, implement isOpenBracket
bool isOpenBracket(Token token)
{
	return true;
}
// TODO, implement isClosedBracket
bool isClosedBracket(Token token)
{
	return true;
}

std::vector<Token> SlateContext::shuntingYard(std::vector<Token>& tokens)
{
	int output_idx = 0;
	std::vector<Token> output;
	std::vector<Token> operators;

	for (Token token : tokens)
	{
		
		if (isOperand(token))
		{
			output[output_idx++] = token;
		}
		else if (isOperator(token))
		{
			while(!empty(operators) && isOperator(operators.back()) && rightAssociative(token) &&  
			((rightAssociative(token) && precedence(token) < precedence(operators.back())) ||
                    (!rightAssociative(token) && precedence(token) <= precedence(operators.back()))))
			{
                output.push_back(operators.back());
				operators.pop_back();
			}

			operators.push_back(token);
		}
		else if (isOpenBracket(token))
		{
			operators.push_back(token);
		}
		else if (isClosedBracket(token))
		{
			while (!empty(operators) && !isOpenBracket(operators.back()))
			{
				output.push_back(operators.back());
				operators.pop_back();
			}
			if (!empty(operators) && isOpenBracket(operators.back()))
			{
				operators.pop_back();
			}
		}
	}
	while (!empty(operators))
	{
		output.push_back(operators.back());
		operators.pop_back();
	}

	return output;
}

ParseError SlateContext::processSyntaxLine(std::string& line) {
	std::vector<Token> tokens;

	ParseError e = lexer(line, tokens);

	return { 0,"" };

}

void SlateContext::removeExpresion(size_t index) {
	delete expresions[index];
	expresions.erase(expresions.begin() + index);
}

void SlateContext::brk() {
	int a = 5;
}
