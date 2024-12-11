#pragma once
#include "Lexer.h"

namespace SlateLanguage {
	namespace AST {
		namespace NodeTypes {
			enum NodeTypes {
				F, // FUNCTION
				J, // JOIN
				Q, // REVERSE JOIN
				C, // CONSTANT
				U,  // UNKNOWN
				MARKER
			};
		}

		typedef size_t NodeType;

		class Node {
		public:
			NodeType type;
			std::vector<Node*> head;
			std::vector<Node*> tail;
		};

		class Marker : public Node {
		public:
			SlateLanguage::Lexer::MarkerType marker;
			Marker(SlateLanguage::Lexer::MarkerType marker) {
				this->type = NodeTypes::MARKER;
				this->marker = marker;
			}
		};

		class FNode : public Node {
		public:
			Function* function;
			FNode(Function* function) {
				this->type = NodeTypes::F;
				this->function = function;
			}
		};

		class JNode : public Node {
		public:
			size_t nestingLevel;
			JNode(size_t nestingLevel) {
				this->type = NodeTypes::J;
				this->nestingLevel = nestingLevel;
			}
		};

		class QNode : public Node {
		public:
			QNode() {
				this->type = NodeTypes::Q;
			}
		};

		class CNode : public Node {
		public:
			Object* constant;
			CNode(Object* constant) {
				this->type = NodeTypes::C;
				this->constant = constant;
			}
		};

		class UNode : public Node {
		public:
			std::string name;
			UNode(std::string name) {
				this->type = NodeTypes::U;
				this->name = name;
			}
		};

		extern void printNode(Node* n, Definitions& definitions, size_t spaces = 0);
	}

	namespace Parser {

		extern AST::Node* parser(std::vector<SlateLanguage::Lexer::Token*>& wrappers);
	}
}