#pragma once

#include <vector>
#include <string>
#include "SlateErrors.h"

namespace SlateLanguage {
	namespace Tokenizer {
		typedef size_t BasicTokenType;

		namespace BasicTokenTypes {
			enum BasicTokenTypes {
				NUMERICAL_CONSTANT,		// Constant real numbers
				SYMBOL,					// Variable/function name
				BEGIN_SCOPE,			// (
				END_SCOPE,				// )

				END_COUNT
			};
		}

		struct BasicToken {
		public:
			BasicTokenType type;
			StringLocation location;
			BasicToken(BasicTokenType type, size_t begin, size_t end) {
				this->type = type;
				location = { begin,end };
			}
		};

		extern bool isAtoZ(char c);
		/*
		* @param c Input charracter
		* @return Retruns TRUE if c is a char between 0 and 9
		*/
		extern bool is0to9(char c);
		extern bool isControlSeqeuenceCharacter(std::string s);
		/*
		* @return Returns TRUE if the TeX symbol can be the base of a character
		*/
		extern bool isSymbolBase(std::string s);
		/*
		* @return Returns TRUE if the TeX symbol can be the flare of a character
		*/
		extern bool isSymbolFlare(std::string s);
		/*
		* @return Returns TRUE if the TeX symbol is possible binary operator
		*/
		extern bool isBinaryOperator(std::string s);
		extern bool isControlSequenceFunction(std::string s);

		extern void tokenizer(std::string& line, std::vector<BasicToken>& tokens);
	}
}