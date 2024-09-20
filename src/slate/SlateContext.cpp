#include "SlateContext.h"
#include "ParseState.h"
#include "SlateDefinitions.h"
#include <format>
#include "SlateErrors.h"
#include <iostream>

ExpressionInfo SlateContext::newExpression() {
	expresions.push_back(new std::string(""));
	return ExpressionInfo( *(expresions[expresions.size() - 1]), expresions.size() - 1 );
}

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
	if (i > 0 && s[i-1]==' ') {
		while (i > 0 && s[i] == ' ') i--;
	}
}

bool isSymbolBase(std::string s) {
	return std::find(SlateDefinitions::symbolBases.begin(), SlateDefinitions::symbolBases.end(), s) != SlateDefinitions::symbolBases.end();
}

bool isSymbolFlare(std::string s) {
	return std::find(SlateDefinitions::symbolFlares.begin(), SlateDefinitions::symbolFlares.end(), s) != SlateDefinitions::symbolFlares.end();
}

void SlateContext::parse() {
	for (std::string* s : expresions) {
		parseLine(*s);
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
	return line.size() == i;
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

ParseError lexer(std::string& line, std::vector<Token>& tokens) {
	size_t begin = 0;
	size_t end = 0;
	size_t i = 0;


	for (; i < line.size();) {
		std::string current = peek(line, i);
		if (current.size() == 0) return OK; // It means that all remaining text is just white spaces
		if (isSymbolBase(current)) {
			begin = jump(line, i);
			if (!isEnd(line, i) && peek(line, i) == "_") {
				jump(line, i);
				end = i;
				if (isEnd(line, i)) return EMPTY_SUBSCRIPT;
				if (peek(line, i) == "{") {
					jump(line, i);
					end = i;
					if (isEnd(line, i)) return UNCLOSED_SUBSCRIPT;
					if (peek(line, i) == "}") return EMPTY_SUBSCRIPT;
					if (!iterateOverBrackets(line,i)) return UNCLOSED_SUBSCRIPT;
					end = i;
				}
				else {
					if (isSymbolFlare(peek(line, i))) {
						jump(line, i);
						end = i;
						if (isEnd(line, i)) return EMPTY_FLARE;
						if (peek(line, i) == "{") {
							jump(line, i);
							end = i;
							if (isEnd(line, i)) return UNCLOSED_FLARE;
							if (peek(line, i) == "}") return EMPTY_FLARE;
							if (!iterateOverBrackets(line,i)) return UNCLOSED_FLARE;
							end = i;
						}
					}
					else {
						jump(line, i);
						end = i;
					}
				}
			}
			else end = i;

			tokens.push_back(Token(SYMBOL, begin, end));
		}
	}
	return OK;
}

ParseError SlateContext::parseLine(std::string& line) {
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
