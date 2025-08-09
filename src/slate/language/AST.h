#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "SlateErrors.h"

#include "../objects/Object.h"
#include "../objects/function/Function.h"

#define MAX_NODE_FLAG_COUNT 4

namespace SlateLanguage {
	namespace AST {
		namespace NodeTypes {
			enum NodeTypes {
				F,		// FUNCTION
				J,		// JOIN
				Q,		// REVERSE JOIN
				C,		// CONSTANT
				U,		// UNKNOWN
				MARKER	// MARKER
			};
		}

		typedef size_t NodeType;

		class Node {
    private:
      bool flags[MAX_NODE_FLAG_COUNT];
		public:
			StringLocation debugLocation;
			NodeType type;
			std::vector<Node*> head;
			std::vector<Node*> tail;

      bool getFlag(int f) {
        return flags[f];
      }

      void setFlags(int f, bool s) {
        flags[f] = s;
      }

      bool contains(Node* other) {
        if (this == other) return true;
        for (size_t i = 0;i < tail.size();i++) {
          if (tail[i]->contains(other)) return true;
        }
        return false;
      }

      bool anyNull() {
        for (size_t i = 0;i < this->tail.size();i++) {
          if (this->tail[i] == nullptr) return true;
          if (this->tail[i]->anyNull()) return true;
        }
        return false;
      }

      size_t size() {
        size_t total = 1;
        for (size_t i = 0;i < tail.size();i++) {
          if (tail[i] != nullptr) total += tail[i]->size();
          else total++;
        }
        return total;
      }

      std::vector<Node*> leafs() {
        std::vector<Node*> l;
        if (tail.size() == 0) {
          l.push_back(this);
          return l;
        }
        for (size_t i = 0;i < tail.size();i++) {
          if (tail[i] != nullptr) {
            std::vector<Node*> other = tail[i]->leafs();
            for (size_t j = 0;j < other.size();j++) l.push_back(other[j]);
          }
        }
        return l;
      }

      void clearFlags() {
        for (size_t i = 0;i < MAX_NODE_FLAG_COUNT;i++) {
          flags[i] = false;
        }
        for (size_t i = 0;i < tail.size();i++) {
          if (tail[i] != nullptr) tail[i]->clearFlags();
        }
      }
       
		};

		class Marker : public Node {
		public:
			size_t marker;
			Marker(size_t marker) {
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

    extern Node* treeTop(Node* head);
		extern Node* copyTree(Node* head);
    extern Node* copyTreeWhole(Node* head, std::vector<size_t> path = std::vector<size_t>());
	}
}
