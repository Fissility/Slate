#pragma once
#include "Tokenizer.h"
#include "../objects/Object.h"
#include "../SlateDefinitions.h"

namespace SlateLanguage {
	namespace Lexer {
		namespace TokenTypes {
			enum SyntaxWrapperTypes {
				KNOWN,
				UNKNOWN,
				MARKER
			};
		}

		namespace MarkerTypes {
			enum MarkerTypes {
				BEGIN_SCOPE,
				END_SCOPE,
				COLON,
				COMMA
			};
		}

		typedef size_t TokenType;
		typedef size_t MarkerType;

		class Token {

		public:

			TokenType type;
			// This is needed for error throwing so in the later stages errors can still point to a location in the initial string
			size_t nestingLevel = 0;

			StringLocation location;

			Token(TokenType type, size_t nestingLevel) {
				this->type = type;
				this->nestingLevel = nestingLevel;
			}

		};

		namespace KnownKinds {
			enum KnownKinds {
				OPERATOR,
				BINARY_OPERATOR,
				BINARY_OPERATOR_UNARY,
				OPERAND
			};
		}

		typedef size_t KnownKind;

		class KnownToken : public Token {
		public:

			Object* o;

			KnownKind kind;

			KnownToken(Object* o, KnownKind kind, StringLocation location, size_t nestingLevel) : Token(TokenTypes::KNOWN, nestingLevel) {
				this->o = o;
				this->kind = kind;
				this->location = location;
			}

		};

		class UnknownToken : public Token {

		public:

			std::string name;
			bool canBeFunctionOrExpression = false;

			UnknownToken(std::string name, StringLocation location, size_t nestingLevel) : Token(TokenTypes::UNKNOWN, nestingLevel) {
				this->name = name;
				this->location = location;
			}

		};

		// Not quite operators
		class MarkerToken : public Token {
		public:

			MarkerType mType;

			MarkerToken(MarkerType mType, StringLocation location, size_t nestingLevel) : Token(TokenTypes::MARKER, nestingLevel) {
				this->mType = mType;
				this->location = location;
			}

		};


		extern bool isOperator(Token* wrapper);
		extern bool isOpenBracket(Token* wrapper);
		extern bool isClosedBracket(Token* wrapper);
		extern bool isOperand(Token* wrapper);
		extern bool isConstant(Token* wrapper);
		extern bool isUnknown(Token* wrapper);
		extern bool isFunction(Token* wrapper);
		extern bool isComma(Token* wrapper);
		extern bool isBinaryOperator(Token* wrapper);

		extern void lexer(std::string line, Definitions& definitions, std::vector<Token*>& objects);
	}
}