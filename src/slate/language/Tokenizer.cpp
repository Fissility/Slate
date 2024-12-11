#include "Tokenizer.h"
#include "SlateErrors.h"
#include "../SlateDefinitions.h"

// Helper functions for the tokenizer

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
* @param s The string in which it should be searched
* @param i The current position index inside the string
* @return Returns the control sequence present at a specific position in the string, if none is there it returns an empty string
*/
std::string getNextTeX(std::string& s, size_t& i) {
	if (s[i] != '\\') return "";
	std::string name = "\\";
	i++;
	if (SlateLanguage::Tokenizer::isControlSeqeuenceCharacter(std::string(1, s[i])) && i < s.size()) {
		name += s[i];
		return name;
	}
	while (true) {
		if ((SlateLanguage::Tokenizer::isAtoZ(s[i]) || SlateLanguage::Tokenizer::is0to9(s[i])) && i < s.size()) name += s[i];
		else {
			i--;
			break;
		}
		i++;
	}
	return name;
}

/*
* @param s The string in which the next term should be searched in
* @param index The index position inside the string
* @return Returns and advances to the next syntactically indivisble structure in the string. Either a character or a control sequence.
*/
size_t jump(std::string& s, size_t& index) {
	skipWhiteSpaces(s, index);
	size_t begin = index;
	if (SlateLanguage::Tokenizer::is0to9(s[index])) {
		index++;
		while (index < s.size() && (SlateLanguage::Tokenizer::is0to9(s[index]) || s[index] == '.')) index++;
	}
	else if (s[index] != '\\') {
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
	if (SlateLanguage::Tokenizer::is0to9(s[index])) {
		std::string result = "";
		result += s[index++];
		// TODO: option to change the decimal delimitator between . and ,
		while (index < s.size() && (SlateLanguage::Tokenizer::is0to9(s[index]) || s[index] == '.')) {
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
		if (start >= line.size()) return false;
		s = line[++start];
		if (s == '{' && !ignoreNext) openCount++;
		if (s == '}' && !ignoreNext) openCount--;
		ignoreNext = false;
		if (s == '\\') ignoreNext = true;
	}
	start++;
	return true;
}

bool isGroupClosed(std::string& line, size_t index) {
	return iterateOverBrackets(line, index);
}

bool isGroupOpened(std::string& line, size_t index) {
	size_t openCount = 1;
	char s;
	bool ignoreNext = false;
	while (openCount != 0) {
		if (index == 0) return false;
		s = line[--index];
		if (s == '{' && !ignoreNext) openCount--;
		if (s == '}' && !ignoreNext) openCount++;
		ignoreNext = false;
		if (s == '\\') ignoreNext = true;
	}
	return true;
}

bool doesLeftEnd(std::string& line, size_t index) {
	size_t count = 1;
	while (count != 0) {
		if (isEnd(line, index)) return false;
		std::string n = peek(line, index);
		if (n == "\\left") count++;
		if (n == "\\right") count--;
		jump(line, index);
	}
	return true;
}

bool doesRightStart(std::string& line, size_t index) {
	return true; // TODO: implement this
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
		if (isEnd(line, i)) throw CompileUnclosedGroup(begin, i);
		if (peek(line, i) == "}") throw CompileEmptySubscript(begin, i);
		if (!iterateOverBrackets(line, i)) throw CompileUnclosedGroup(begin, i);
	}
	// Check if the subscript is done using [...]_\left([...]\right)
	// Does not also check if there is a bracket after it as that would not be valid TeX
	else if (peek(line, i) == "\\left") {
		begin = jump(line, i);
		bool passedRight = false;
		bool finished = false;
		while (!finished) {
			if (isEnd(line, i)) throw CompileUnclosedGroup(begin, i);
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
		if (SlateLanguage::Tokenizer::isSymbolFlare(peek(line, i))) {
			begin = jump(line, i);
			if (isEnd(line, i)) throw CompileEmptyFlare(begin, i);
			if (peek(line, i) == "{") {
				begin = jump(line, i);
				if (isEnd(line, i)) throw CompileUnclosedGroup(begin, i);
				if (peek(line, i) == "}") throw CompileEmptyFlare(begin, i);
				if (!iterateOverBrackets(line, i)) throw CompileUnclosedGroup(begin, i);
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

void pushOverride(StringLocation location, std::vector<SlateLanguage::Tokenizer::BasicToken>& tokenList, std::vector<SlateLanguage::Tokenizer::BasicToken>& tokenOverrides) {
	for (size_t i = 0; i < tokenOverrides.size(); i++) {
		StringLocation ol = tokenOverrides[i].location;
		if (location == ol) {
			tokenList.push_back(tokenOverrides[i]);
			tokenOverrides.erase(tokenOverrides.begin() + i);
			return;
		}
	}
}

void pushToken(SlateLanguage::Tokenizer::BasicToken t, std::vector<SlateLanguage::Tokenizer::BasicToken>& tokenList, std::vector<SlateLanguage::Tokenizer::BasicToken>& tokenOverrides) {
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

// Definitions for SlateLanguage::Tokenizer

/*
* @param c Input charracter
* @return Retruns TRUE if c is a char between a and z or A and Z
*/
bool SlateLanguage::Tokenizer::isAtoZ(char c) {
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

/*
* @param c Input charracter
* @return Retruns TRUE if c is a char between 0 and 9
*/
bool SlateLanguage::Tokenizer::is0to9(char c) {
	return '0' <= c && c <= '9';
}

bool SlateLanguage::Tokenizer::isControlSeqeuenceCharacter(std::string s) {
	return std::find(SlateDefinitions::controlSeqeuenceCharacters.begin(), SlateDefinitions::controlSeqeuenceCharacters.end(), s) != SlateDefinitions::controlSeqeuenceCharacters.end();
}

/*
* @return Returns TRUE if the TeX symbol can be the base of a character
*/
bool SlateLanguage::Tokenizer::isSymbolBase(std::string s) {
	return std::find(SlateDefinitions::symbolBases.begin(), SlateDefinitions::symbolBases.end(), s) != SlateDefinitions::symbolBases.end();
}

/*
* @return Returns TRUE if the TeX symbol can be the flare of a character
*/
bool SlateLanguage::Tokenizer::isSymbolFlare(std::string s) {
	return std::find(SlateDefinitions::symbolFlares.begin(), SlateDefinitions::symbolFlares.end(), s) != SlateDefinitions::symbolFlares.end();
}

/*
* @return Returns TRUE if the TeX symbol is possible binary operator
*/
bool SlateLanguage::Tokenizer::isBinaryOperator(std::string s) {
	return std::find(SlateDefinitions::binaryOperators.begin(), SlateDefinitions::binaryOperators.end(), s) != SlateDefinitions::binaryOperators.end();
}

bool SlateLanguage::Tokenizer::isControlSequenceFunction(std::string s) {
	return SlateDefinitions::controlSequenceFunctions.count(s);
}

void SlateLanguage::Tokenizer::tokenizer(std::string& line, std::vector<BasicToken>& tokens) {
	size_t begin = 0;
	size_t end = 0;
	size_t i = 0;

	std::vector<BasicToken> tokenOverrides;

	while (!isEnd(line, i)) {
		std::string current = peek(line, i);

		if (current == "{") {
			begin = jump(line, i);
			end = i;
			if (!isGroupClosed(line, begin)) throw CompileUnclosedGroup(begin, end);
			pushOverride({ begin,end }, tokens, tokenOverrides);
			continue;
		}

		if (current == "}") {
			begin = jump(line, i);
			end = i;
			if (!isGroupOpened(line, begin)) throw CompileUnopenedGroup(begin, end);
			pushOverride({ begin,end }, tokens, tokenOverrides);
			continue;
		}

		if (current == "\\left") {
			begin = jump(line, i);
			end = i;
			if (!doesLeftEnd(line, end)) throw CompileUnclosedGroup(begin, end);
			continue;
		}

		if (current == "\\right") {
			begin = jump(line, i);
			end = i;
			if (!doesRightStart(line, begin)) throw CompileUnopenedGroup(begin, end);
			continue;
		}

		if (current == ",") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(BasicToken(BasicTokenTypes::SYMBOL, begin, end));
			continue;
		}

		if (is0to9(current[0])) {
			begin = jump(line, i);
			end = i;
			tokens.push_back(BasicToken(BasicTokenTypes::NUMERICAL_CONSTANT, begin, end));
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
						if (scopeIndex == 0) throw CompileUnopenedGroup(localBegin, localEnd);
						scopeIndex--;
						if (scopeIndex == 0) {
							remainingCtrlSqArguments--;
							if (remainingCtrlSqArguments == 0) {
								tokenOverrides.push_back(BasicToken(BasicTokenTypes::END_SCOPE, localBegin, localEnd));
							}
							else {
								tokenOverrides.push_back(BasicToken(BasicTokenTypes::SYMBOL, localBegin, localEnd));
							}
						}
					}
				}
				tokens.push_back(BasicToken(BasicTokenTypes::SYMBOL, begin, end));
				tokens.push_back(BasicToken(BasicTokenTypes::BEGIN_SCOPE, end, end));
			}
			else {
				if (peek(line, i) == "_") {
					jump(line, i);
					end = i;
					processSubscript(line, i, begin);
					end = i;
				}
				else end = i;
				tokens.push_back(BasicToken(BasicTokenTypes::SYMBOL, begin, end));
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
				if (isEnd(line, i)) throw CompileUnclosedGroup(begin, end);
				if (peek(line, i) == "}") throw CompileEmptyFlare(begin, end);
				if (!iterateOverBrackets(line, i)) throw CompileUnclosedGroup(begin, end);
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
			tokens.push_back(BasicToken(BasicTokenTypes::SYMBOL, begin, end));
			continue;
		}

		// OPEN BRACKET CHECK
		if (current == "(") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(BasicToken(BasicTokenTypes::BEGIN_SCOPE, begin, end));
			continue;
		}


		// CLOSED BRACKET CHECK
		if (current == ")") {
			begin = jump(line, i);
			end = i;
			tokens.push_back(BasicToken(BasicTokenTypes::END_SCOPE, begin, end));
			continue;
		}

		throw CompileOutOfPlace(begin, end);
	}
}