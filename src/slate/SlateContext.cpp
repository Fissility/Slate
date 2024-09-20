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

std::string getNextTeX(std::string& s, size_t& i) {
	if (s[i] != '\\') return "";
	std::string name = "\\";
	while (true) {
		i++;
		if (isAtoB(s[i]) && i < s.size()) name += s[i];
		else {
			i--;
			break;
		}
	}
	return name;
}

void skipWhiteSpaces(std::string& s, size_t& i) {
	while (s[i] == ' ' && i+1 < s.size()) i++;
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

bool increment(std::string& s, size_t& index) {
	index++;
	if (index > s.size()) return false;
	return true;
}

std::string getNextIndivisible(std::string& s, size_t& index) {
	skipWhiteSpaces(s, index);
	if (s[index] != '\\') {
		std::string result(1, s[index]);
		return result;
	}
	std::string result = getNextTeX(s, index);
	return result;
}

std::string& advance(std::string& current, std::string& line, size_t& index) {
	current = getNextIndivisible(line,index);
	return current;
}

bool isEnd(std::string& line, size_t& i) {
	return line.size() == i;
}

ParseError lexer(std::string& line, std::vector<Token>& tokens) {
	ParseState state = ParseStates::BEGIN;
	size_t begin = 0;
	size_t end = 0;

	std::string current;
	size_t i = 0;

	for (; i < line.size();) {
		advance(current, line, i);
		if (isSymbolBase(current)) {
			end = i;
			if (!isEnd(line, i) && advance(current, line, i) == "_") {
				end = i;
				if (isEnd(line, i)) return EMPTY_SUBSCRIPT;
				if (advance(current, line, i) == "{") {
					end = i;
					if (isEnd(line, i)) return UNCLOSED_SUBSCRIPT;
					if (advance(current, line, i) == "}") return EMPTY_SUBSCRIPT;
					while (current != "}") {
						if (isEnd(line, i)) return UNCLOSED_SUBSCRIPT;
						advance(current, line, i);
					}
					end = i;
				}
				else {
					if (isSymbolFlare(current)) {
						if (advance(current, line, i) == "{") {
							if (advance(current, line, i) == "}") return EMPTY_FLARE;
							while (current != "}") {
								if (isEnd(line, i)) return UNCLOSED_FLARE;
								advance(current, line, i);
							}
							end = i;
						}
					}
				}
			}

			tokens.push_back(Token(SYMBOL, begin, end));
			begin = end + 1;
			i = end + 1;
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
