#pragma once
#include "Tokenizer.h"
#include "../objects/Object.h"
#include "../SlateDefinitions.h"

namespace SlateLanguage {
	namespace Lexer {
		namespace SyntaxWrapperTypes {
			enum SyntaxWrapperTypes {
				KNOWN,
				DEPENDENT,
				UNKNOWN,
				MARKER
			};
		}

		namespace MarkerTypes {
			enum MarkerTypes {
				BEGIN_SCOPE,
				END_SCOPE,
				EQUALS,
				COLON,
				COMMA
			};
		}

		typedef size_t SyntaxWrapperType;
		typedef size_t MarkerType;

		class ObjectSyntaxWrapper {

		public:

			SyntaxWrapperType type;
			// This is needed for error throwing so in the later stages errors can still point to a location in the initial string
			size_t nestingLevel = 0;

			StringLocation location;

			ObjectSyntaxWrapper(SyntaxWrapperType type, size_t nestingLevel) {
				this->type = type;
				this->nestingLevel = nestingLevel;
			}

		};

		namespace KnownKinds {
			enum KnownKinds {
				OPERATOR,
				BINARY_OPERATOR,
				OPERAND
			};
		}

		typedef size_t KnownKind;

		class Known : public ObjectSyntaxWrapper {
		public:

			Object* o;

			KnownKind kind;

			Known(Object* o, KnownKind kind, StringLocation location, size_t nestingLevel) : ObjectSyntaxWrapper(SyntaxWrapperTypes::KNOWN, nestingLevel) {
				this->o = o;
				this->kind = kind;
				this->location = location;
			}

		};

		class Unknown : public ObjectSyntaxWrapper {

		public:

			std::string name;
			bool canBeFunctionOrExpression = false;

			Unknown(std::string name, StringLocation location, size_t nestingLevel) : ObjectSyntaxWrapper(SyntaxWrapperTypes::UNKNOWN, nestingLevel) {
				this->name = name;
				this->location = location;
			}

		};

		// Not quite operators
		class Marker : public ObjectSyntaxWrapper {
		public:

			MarkerType mType;

			Marker(MarkerType mType, StringLocation location, size_t nestingLevel) : ObjectSyntaxWrapper(SyntaxWrapperTypes::MARKER, nestingLevel) {
				this->mType = mType;
				this->location = location;
			}

		};


		extern bool isOperator(ObjectSyntaxWrapper* wrapper);
		extern bool isOpenBracket(ObjectSyntaxWrapper* wrapper);
		extern bool isClosedBracket(ObjectSyntaxWrapper* wrapper);
		extern bool isOperand(ObjectSyntaxWrapper* wrapper);
		extern bool isConstant(ObjectSyntaxWrapper* wrapper);
		extern bool isUnknown(ObjectSyntaxWrapper* wrapper);
		extern bool isFunction(ObjectSyntaxWrapper* wrapper);
		extern bool isComma(ObjectSyntaxWrapper* wrapper);
		extern bool isEquals(ObjectSyntaxWrapper* wrapper);
		extern bool isBinaryOperator(ObjectSyntaxWrapper* wrapper);

		extern void lexer(std::string line, Definitions& definitions, std::vector<ObjectSyntaxWrapper*>& objects);
	}
}