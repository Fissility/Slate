#include "AST.h"

using namespace SlateLanguage::AST;

Node* SlateLanguage::AST::copyTree(Node* head) {
	Node* copy = nullptr;
	switch (head->type) {
		case NodeTypes::C: {
			copy = new CNode(((CNode*)head)->constant);
			break;
		}
		case NodeTypes::F: {
			copy = new FNode(((FNode*)head)->function);
			break;
		}
		case NodeTypes::J: {
			copy = new JNode(((JNode*)head)->nestingLevel);
			break;
		}
		case NodeTypes::MARKER: {
			copy = new Marker(((Marker*)head)->marker);
			break;
		}
		case NodeTypes::Q: {
			copy = new QNode();
			break;
		}
		case NodeTypes::U: {
			copy = new UNode(((UNode*)head)->name);
			break;
		}
	}
	for (Node* t : head->tail) {
		Node* tCopy = copyTree(t);
		copy->tail.push_back(tCopy);
		tCopy->head.push_back(copy);
	}
    return copy;
}
