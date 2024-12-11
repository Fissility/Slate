#include "Lexer.h"
#include "../objects/number/Number.h"

using namespace SlateLanguage::Tokenizer;

bool SlateLanguage::Lexer::isOperator(Token* wrapper) {
	if (wrapper->type == TokenTypes::MARKER) {
		MarkerToken* marker = ((MarkerToken*)wrapper);
		if (marker->mType == MarkerTypes::EQUALS || marker->mType == MarkerTypes::COLON || marker->mType == MarkerTypes::COMMA) return true;
	}
	if (wrapper->type == TokenTypes::KNOWN) {
		KnownToken* i = (KnownToken*)wrapper;
		return i->kind == KnownKinds::OPERATOR || i->kind == KnownKinds::BINARY_OPERATOR;
	}
	return false;
}

bool SlateLanguage::Lexer::isOpenBracket(Token* wrapper) {
	return wrapper->type == TokenTypes::MARKER && ((MarkerToken*)wrapper)->mType == MarkerTypes::BEGIN_SCOPE;
}

bool SlateLanguage::Lexer::isClosedBracket(Token* wrapper) {
	return wrapper->type == TokenTypes::MARKER && ((MarkerToken*)wrapper)->mType == MarkerTypes::END_SCOPE;
}

bool SlateLanguage::Lexer::isFunction(Token* wrapper) {
	if (wrapper->type != TokenTypes::KNOWN) return false;
	return((KnownToken*)wrapper)->kind == KnownKinds::OPERATOR;
}

// TODO, implement isOperand
bool SlateLanguage::Lexer::isOperand(Token* wrapper) {
	return !isOperator(wrapper) && !isOpenBracket(wrapper) && !isClosedBracket(wrapper);
}

bool SlateLanguage::Lexer::isConstant(Token* wrapper) {
	if (wrapper->type != TokenTypes::KNOWN) return false;
	return((KnownToken*)wrapper)->kind == KnownKinds::OPERAND;
}

bool SlateLanguage::Lexer::isUnknown(Token* wrapper) {
	return wrapper->type == TokenTypes::UNKNOWN;
}

bool SlateLanguage::Lexer::isComma(Token* wrapper) {
	if (wrapper->type != TokenTypes::MARKER) return false;
	return((MarkerToken*)wrapper)->mType == MarkerTypes::COMMA;
}

bool SlateLanguage::Lexer::isEquals(Token* wrapper) {
	if (wrapper->type != TokenTypes::MARKER) return false;
	return((MarkerToken*)wrapper)->mType == MarkerTypes::EQUALS;
}

bool SlateLanguage::Lexer::isBinaryOperator(Token* wrapper) {
	if (wrapper->type != TokenTypes::KNOWN) return false;
	return((KnownToken*)wrapper)->kind == KnownKinds::BINARY_OPERATOR;
}

void SlateLanguage::Lexer::lexer(std::string line, Definitions& definitions, std::vector<Token*>& objects) {
	std::vector<SlateLanguage::Tokenizer::BasicToken> basicTokens;

	tokenizer(line, basicTokens);

	for (size_t i = 0; i < basicTokens.size(); i++) {
		BasicToken& token = basicTokens[i];
		size_t nestingLevel = 0;
		for (size_t j = 0; j <= i; j++) {
			if (basicTokens[j].type == BasicTokenTypes::BEGIN_SCOPE) nestingLevel++;
			//                                             V This is needed rn, TODO: move bracket detectin upper in the chain
			if (basicTokens[j].type == BasicTokenTypes::END_SCOPE && nestingLevel != 0) nestingLevel--;
		}
		switch (token.type) {
			case BasicTokenTypes::NUMERICAL_CONSTANT: {
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
				objects.push_back(new KnownToken(new Number(result), KnownKinds::OPERAND, token.location, nestingLevel));
				break;
			}
			case BasicTokenTypes::SYMBOL: {
				// Signifies it is just a name, it can either be some external varaible or a function
				std::string name = line.substr(token.location.begin, token.location.end - token.location.begin);

				if (name == "=") {
					objects.push_back(new MarkerToken(MarkerTypes::EQUALS, token.location, nestingLevel));

				}
				else if (name == ":") {
					objects.push_back(new MarkerToken(MarkerTypes::COLON, token.location, nestingLevel));
				}
				else if (name == "," || name == "}") {
					objects.push_back(new MarkerToken(MarkerTypes::COMMA, token.location, nestingLevel));
				}
				else if (definitions.definitionExists(name)) {

					Object* o = definitions.getDefinition(name);
					// If the next token is the begining of a scope and the object linked to the current token is a function
					// Then it means that the evaluation of a function is taking place
					// V V
					// f (...)
					if (i != basicTokens.size() - 1 && basicTokens[i + 1].type == BasicTokenTypes::BEGIN_SCOPE && o->type == Types::FUNCTION) {
						objects.push_back(new KnownToken(definitions.getDefinition(name), KnownKinds::OPERATOR, token.location, nestingLevel));
					}
					else if (o->type == Types::BINARY_OPERATOR) {
						KnownKind kind;
						if (objects.empty() || (!isOperand(objects.back()) && !isClosedBracket(objects.back()))) {
							if (((BinaryOperator*)definitions.getDefinition(name))->canBeUnary && i != basicTokens.size() - 1 && (basicTokens[i + 1].type == BasicTokenTypes::SYMBOL || basicTokens[i + 1].type == BasicTokenTypes::NUMERICAL_CONSTANT)) {
								kind = KnownKinds::OPERATOR;
							}
							else {
								kind = KnownKinds::OPERAND;
							}
						}
						else kind = KnownKinds::BINARY_OPERATOR; // It means it is an operator


						objects.push_back(new KnownToken(definitions.getDefinition(name), kind, token.location, nestingLevel));
					}
					// Otherwise if it is a function then it used as an operand (f+g,f-g,f\\circ g) or it isn't a function in which case it is a variable
					else {
						objects.push_back(new KnownToken(definitions.getDefinition(name), KnownKinds::OPERAND, token.location, nestingLevel));
					}
				}
				else {
					UnknownToken* u = new UnknownToken(name, token.location, nestingLevel);
					if (i != basicTokens.size() - 1 && basicTokens[i + 1].type == BasicTokenTypes::BEGIN_SCOPE) {
						u->canBeFunctionOrExpression = true;
					}
					objects.push_back(new UnknownToken(name, token.location, nestingLevel));
				}
				break;
			}
			case BasicTokenTypes::BEGIN_SCOPE: {

				// This means if the last thing was a function mark that function as on operator
				// If a function is not followed by ( as in, f(..) then it is assumed that the function is not evaluated here
				// and instead it is treated as an operand, for example f+g, f-g, f \circ g
				if (!objects.empty()) {
					Token* ow = objects.back();
					if (ow->type == TokenTypes::KNOWN) {
						KnownToken* ind = (KnownToken*)ow;
						Object* o = ind->o;
						if (o->type == Types::FUNCTION) {
							ind->kind = KnownKinds::OPERATOR;
						}
					}
				}

				objects.push_back(new MarkerToken(MarkerTypes::BEGIN_SCOPE, token.location, nestingLevel));
				break;
			}
			case BasicTokenTypes::END_SCOPE: {
				objects.push_back(new MarkerToken(MarkerTypes::END_SCOPE, token.location, nestingLevel));
				break;
			}
		}
	}
}
