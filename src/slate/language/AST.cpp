#include "AST.h"

using namespace SlateLanguage::AST;

Node* SlateLanguage::AST::treeTop(Node* head) {
  if (head == nullptr) return nullptr;
  if (head->head.size() == 0) return head;
  else return treeTop(head->head[0]);
}

Node* SlateLanguage::AST::copyTree(Node* head) {
  if (head == nullptr) return nullptr;
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
		if (tCopy != nullptr) tCopy->head.push_back(copy);
	}
  for (size_t i = 0;i < MAX_NODE_FLAG_COUNT;i++) {
    copy->setFlags(i, head->getFlag(i));
  }
  return copy;
}

Node* SlateLanguage::AST::copyTreeWhole(Node* head, std::vector<size_t> path) {
  if (head->head.size() != 0) {
    for (size_t i = 0;i < head->head[0]->tail.size();i++) {
      if (head->head[0]->tail[i] == head)  {
        path.push_back(i);
        return copyTreeWhole(head->head[0], path);
      }
    }
  } else {
    Node* copy = copyTree(head);
    if (path.size() != 0) {
      for (int i = path.size() - 1;i >= 0;i--) {
        copy = copy->tail[path[i]];
      }
    }
    return copy;
  }
  return nullptr;
}
