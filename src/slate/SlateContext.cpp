#include "SlateContext.h"
#include "SlateDefinitions.h"

#include "language/AST.h"
#include "language/SlateErrors.h"
#include "language/Tokenizer.h"
#include "language/Lexer.h"
#include "language/Parser.h"
#include "objects/Expression.h"
#include "objects/tuple/BiCategory.h" //X
#include <algorithm>
#include <climits>
#include <compare>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <stack>
#include <unordered_map>
#include "SlateType.h"

#define TREE_PIVOT_FLAG 0

using namespace SlateLanguage;

void SlateContext::addDefinitions(Definitions& defs) {
  definitions.addFrom(defs);
}

std::string SlateContext::getObjectName(Object* o) {
	if (definitions.objectHasString(o)) {
		return definitions.getString(o);
	}
	switch (o->type) {
		case Types::NUMBER: {
			return std::to_string(((Number*)o)->value);
		}
		case Types::TUPLE: {
			Tuple* t = (Tuple*)o;
			std::string out = "(";
			for (size_t i = 0; i < t->length; i++) {
				out += getObjectName((*t)[i]);
				if (i != t->length - 1) out += ",";
			}
			out += ")";
			return out;
		}
		default: {
			return "<" + Types::getTypeName(o->type) + ">";
		}
	}
}

// TODO: account that some control sequences don't have standard paramters \frac, \sqrt etc
std::string SlateContext::displayStringFromAST(AST::Node* head) {
	if (head->type == AST::NodeTypes::C) {
		Object* constant = ((AST::CNode*)head)->constant;
		return getObjectName(constant);
	}
	if (head->type == AST::NodeTypes::U) {
		return ((AST::UNode*)head)->name;
	}
	std::vector<std::string> tailStrings;
	for (AST::Node* t : head->tail) tailStrings.push_back(displayStringFromAST(t));
	if (head->type == AST::NodeTypes::F) {
		Function* function = ((AST::FNode*)head)->function;
		if (function->type == Types::BINARY_OPERATOR) {
			return tailStrings[0]; // This case is already processed in the required J Node before it
		}
		else {
			return "\\left(" +getObjectName(function) + tailStrings[0] + "\\right)";
		}
	}
	if (head->type == AST::NodeTypes::J) {
		if (!head->head.empty() && head->head[0]->type == AST::NodeTypes::F && ((AST::FNode*)head->head[0])->function->type == Types::BINARY_OPERATOR) { 
			return "\\left({" + tailStrings[0] + "} " + getObjectName(((AST::FNode*)head->head[0])->function) + " {" + tailStrings[1] + "}\\right)";
		}
		else {
			std::string out = "\\left(";
			for (size_t i = 0; i < tailStrings.size(); i++) {
				out += "{" + tailStrings[i] + "} ";
				if (i != tailStrings.size() - 1) out += ",";
			}
			out += "\\right)";
			return out;
		}
	}
	return "Unable to generate string? This should not happen!";
}

std::function<Object* (Tuple*)> generateExpressionImpl(AST::Node* head, std::vector<std::string>* unknowns) {
	if (head->type == AST::NodeTypes::C) {
		Object* constant = ((AST::CNode*)head)->constant;
		return [=](Tuple* t) {
			return constant;
			};
	}
	if (head->type == AST::NodeTypes::U) {
		std::string unknownName = ((AST::UNode*)head)->name;
		size_t getFrom = unknowns->size();
		for (size_t i = 0; i < unknowns->size(); i++) {
			if ((*unknowns)[i] == unknownName) {
				getFrom = i;
				break;
			}
		}
		if (getFrom == unknowns->size()) unknowns->push_back(unknownName);
		return [=](Tuple* t) {
			return (*t)[getFrom];
		};
	}
	std::vector<std::function<Object* (Tuple*)>> tailImpls;
	for (AST::Node* t : head->tail) tailImpls.push_back(generateExpressionImpl(t, unknowns));
	if (head->type == AST::NodeTypes::F) {
		Function* function = ((AST::FNode*)head)->function;
		std::function<Object* (Tuple*)> input = tailImpls[0];
		return [=](Tuple* t) {
			Object* result = input(t);
			if (!SlateDefinitions::objectIsIn(result, function->domain)) throw RuntimeDomainException(head->debugLocation.begin,head->debugLocation.end);
			return function->evaluate(result);
		};
	}
	if (head->type == AST::NodeTypes::J) {
		size_t inputSize = head->tail.size();
		Object** os = new Object * [inputSize];
		Tuple* joined = new Tuple(inputSize, os);
		return [=](Tuple* t) {
			for (size_t i = 0; i < inputSize; i++) (*joined)[i] = tailImpls[i](t);
			return joined;
		};
	}
}

bool hasUnknowns(AST::Node* node) {
	if (node->type == AST::NodeTypes::U) return true;
	for (AST::Node* t : node->tail) {
		if (hasUnknowns(t)) return true;
	}
	return false;
}

Expression* SlateContext::expressionFromAST(AST::Node* head, std::vector<std::string>* unknowns) {
	std::function<Object* (Tuple*)> impl = generateExpressionImpl(head, unknowns);
	Expression* exp = new Expression(unknowns == nullptr ? 0 : unknowns->size(), impl);
	std::string name = displayStringFromAST(head);
	definitions.registerString(exp, name);
	return exp;
}

void SlateContext::getRepetitionSignature(AST::Node* head, RepetitionMap& map) {

  for (size_t i = 0;i < head->tail.size();i++) {
    getRepetitionSignature(head->tail[i],map);
  }

  for (size_t i = 0;i < map.section.size();i++) {
    if (equalsAST(head,map.section[i])) {
      map.count[i]++;
      for(size_t j = 0;j < i;j++) {
        if (map.count[j] < map.count[i]) {
          size_t temp = map.count[j];
          map.count[j] = map.count[i];
          map.count[i] = temp;
          AST::Node* tempNode = map.section[j];
          map.section[j] = map.section[i];
          map.section[i] = tempNode;
          break;
        }
      }
      return;
    }
  }

  map.section.push_back(head);
  map.count.push_back(1);
}

bool SlateContext::doSignaturesMatch(std::vector<size_t>& expression, std::vector<size_t>& simplification) {
  if (simplification.size() > expression.size()) return false;
  for (size_t i = 0;i < simplification.size();i++) {
    if (expression[i] < simplification[i]) return false;
  }
  return true;
}

bool SlateContext::includesNode(SlateLanguage::AST::Node* node, SlateLanguage::AST::Node* tree) {
  for(size_t i = 0;i < tree->tail.size();i++) {
    if (includesNode(node,tree->tail[i])) return true;
  }
  return equalsNodes(node,tree);
}

bool SlateContext::includesAllBlocks(SlateLanguage::AST::Node* head, SlateLanguage::AST::Node* pattern) {
  if (pattern->type == AST::NodeTypes::U) return true; // Unknowns just mark inputs
  for (size_t i = 0;i < pattern->tail.size();i++) {
    if(!includesAllBlocks(head,pattern->tail[i])) return false;
  }
  return includesNode(pattern,head); 
}

bool SlateContext::heuristicDoSimplification(SlateLanguage::AST::Node* head, Simplification& s, RepetitionMap& mapHead, RepetitionMap& mapSimplification) {
  if (!includesAllBlocks(head,s.from)) return false;
  
  getRepetitionSignature(head,mapHead);
  getRepetitionSignature(s.from, mapSimplification);
  if (!doSignaturesMatch(mapHead.count, mapSimplification.count)) return false;
  return true;
} 

SlateLanguage::AST::Node* SlateContext::toAST(std::string line) {
  std::vector<Lexer::Token*> tokens;
  Lexer::lexer(line, definitions, tokens);
  AST::Node* output = Parser::parser(tokens);
  return output;
}

/*
 * @brief Processes and updates the context with the TeX expression offered in the inputted string.
 * @param line = The string whicb holds the TeX expression
 * @return Returns the relevant objects in relation to the inputted expression. The pointer can also be null if there is no relevant output (e.g. f:A->B doesn't have any relevant output)
 */
Object* SlateContext::interpret(std::string line) {

	AST::Node* output = toAST(line);
	AST::Node* outputCopy = AST::copyTree(output);

	printNode(output);
	std::cout << "\n";
  std::vector<AST::Node*> prv;
  
  //if (reduceTree(outputCopy, outputCopy, prv)) {
  //  printNode(outputCopy);
  //  std::vector<std::string> u;
  //  std::cout << displayStringFromAST(outputCopy) << '\n';
  //} else {
  //  std::cout << "Could not be simplified!\n";
  //}
  //
  //produceMoves(outputCopy,definitions.properties[1], prv);

  for (size_t i = 0;i < prv.size();i++) {
    std::vector<std::string> u;
    printNode(prv[i]);
  }
  std::cout << '\n';

  for (Simplification& s : definitions.simplifications) {
    //std::cout << maybeDoSimplification(output,s) << '\n';
  }

	std::string equals = "=";
	if (output->type == AST::NodeTypes::F && ((AST::FNode*)output)->function == definitions.getDefinition(equals)) {
		AST::Node* tailJ = output->tail[0];
		AST::Node* left = tailJ->tail[0];
		AST::Node* right = tailJ->tail[1];
		if (left->type == AST::NodeTypes::U && !hasUnknowns(right)) {
			Expression* rightExp = expressionFromAST(right, nullptr);
			Object* result = rightExp->evalFunc(nullptr);
			definitions.registerDefinition(((AST::UNode*)left)->name, result);
			return result;
		}
	}
	std::vector<std::string> u;
	return expressionFromAST(output, &u);


	return nullptr;

}

void SlateContext::printNode(AST::Node* n, size_t spaces) {
	for (size_t i = 0; i < spaces; i++) std::cout << "  "; 
  if (n == nullptr) {
    std::cout << "NULL\n";
    return;
  }
  if (n->getFlag(TREE_PIVOT_FLAG)) std::cout << "ANCH ";
	switch (n->type) {
	case AST::NodeTypes::F: {
		std::cout << "F Node " << "(" + getObjectName(((AST::FNode*)n)->function) + ")\n";
		break;
	}
	case AST::NodeTypes::J: {
		std::cout << "J Node\n";
		break;
	}
	case AST::NodeTypes::C: {
		std::cout << "C Node " << "(" + getObjectName(((AST::CNode*)n)->constant) + ")\n";
		break;
	}
	case AST::NodeTypes::U: {
		std::cout << "U Node " << "(\"" + ((AST::UNode*)n)->name + "\")\n";
		break;
	}
	case AST::NodeTypes::MARKER: {
		std::cout << "MARKER\n";
		break;
	}
	}
	for (AST::Node* t : n->tail) printNode(t, spaces + 1);
}

/*
 * @brief Functions used to check wether two AST nodes are equal. There is a caviat in that if a constant node represents an entry to a variable which is stored in the environment then both constants nodes would have to point to the same object in order for any further transformations to not remove the reference from one of the two objects.
 * @param first = First node
 * @param second = Second node
 * @return Returns TRUE if both nodes are equal, FALSE otherwise
 */
bool SlateContext::equalsNodes(SlateLanguage::AST::Node* first, SlateLanguage::AST::Node* second, bool nullPlaceholder) {
  if (first == nullptr || second == nullptr) {
    if (nullPlaceholder) {
      return true;
    } else {
      return first == nullptr && second == nullptr;
    }
  }
	if (first->type != second->type) return false;
	if (first->tail.size() != second->tail.size()) return false;
	switch (first->type) {
	  case AST::NodeTypes::C: {
		  AST::CNode* firstC = (AST::CNode*)first;
		  AST::CNode* secondC = (AST::CNode*)second;

		  // If one of the constants comes from a variable which is already defined in the environment
		  // Then the nodes should only be considered the same if it is actually the same variable
		  if (definitions.objectHasDefinition(firstC->constant) || definitions.objectHasDefinition(secondC->constant)) {
			  return firstC->constant == secondC->constant;
		  }

		  return SlateDefinitions::equals(firstC->constant, secondC->constant);
	  }
	  case AST::NodeTypes::F: {
		  AST::FNode* firstF = (AST::FNode*)first;
		  AST::FNode* secondF = (AST::FNode*)second;
		  return SlateDefinitions::equals(firstF->function, secondF->function);
	  }
	  case AST::NodeTypes::U: {
		  AST::UNode* firstU = (AST::UNode*)first;
		  AST::UNode* secondU = (AST::UNode*)second;
		  return firstU->name == secondU->name;
	  }
	  case AST::NodeTypes::Q:
	  case AST::NodeTypes::J:
		  return first->head.size() == second->head.size() && first->tail.size() && second->head.size();
	  case AST::NodeTypes::MARKER: {
		  AST::Marker* firstM = (AST::Marker*)first;
		  AST::Marker* secondM = (AST::Marker*)second;
		  return firstM->marker == secondM->marker;
	  }
	  default:
		  return false;
	  }
}

bool SlateContext::equalsAST(SlateLanguage::AST::Node* first, SlateLanguage::AST::Node* second, bool nullPlaceholder) {
	if (!equalsNodes(first, second, nullPlaceholder)) return false;
  if (first == nullptr || second == nullptr) return true; // means both were null normally
	for (size_t i = 0; i < first->tail.size(); i++) {
		if (!equalsAST(first->tail[i], second->tail[i], nullPlaceholder)) return false;
	}
	return true;
}

void SlateContext::swapHeads(AST::Node* target, AST::Node* source) {
  source->head = target->head;
  if (!target->head.empty()) {
    for (size_t i = 0;i < target->head[0]->tail.size();i++) {
      if (target->head[0]->tail[i] == target) {
        target->head[0]->tail[i] = source;
      }
    }
  }
}


/*
 * @brief Firstly checks if a certain pattern holds over the inputted AST starting from the head of both. If it holds then it also populates the mapping from the pattern inputs to the inputted AST nodes.
 * @param head = Head of the inputted AST over which the pattern will be checkPatternHead
 * @param inputPattern = Head of the pattern AST
 * @param patternTemplateInputs = Reference to the map in which will populated with the mappings of the pattern to the input AST's nodes if it holds
 * @return Returns TRUE if the pattern holds, FALSE otherwise
 */
bool SlateContext::checkPatternHead(SlateLanguage::AST::Node* head, SlateLanguage::AST::Node* inputPattern, std::unordered_map<std::string, SlateLanguage::AST::Node*>& patternTemplateInputs) {
	if (inputPattern->type == AST::NodeTypes::U) {
		std::string name = ((AST::UNode*)inputPattern)->name;
		if (patternTemplateInputs.find(name) == patternTemplateInputs.end()) {
      //printNode(head);
			patternTemplateInputs[name] = head;
			return true;
		}
		else {
			return equalsAST(patternTemplateInputs[name], head);
		}
	}
	if (!equalsNodes(head, inputPattern)) return false;
	for (size_t i = 0; i < head->tail.size(); i++) {
		if (!checkPatternHead(head->tail[i], inputPattern->tail[i], patternTemplateInputs)) return false;
	}
	return true;
}

void SlateContext::populatePattern(SlateLanguage::AST::Node*& pattern, std::unordered_map<std::string, SlateLanguage::AST::Node*>& patternTemplateInputs, bool throwExpIfNotFound) {

  if (pattern->type == AST::NodeTypes::U) {
    // TODO: free the existing pattern
    pattern = AST::copyTree(patternTemplateInputs[((AST::UNode*)pattern)->name]);
    return;
  }

  for (size_t i = 0; i < pattern->tail.size();i++) {
		if (pattern->tail[i]->type == AST::NodeTypes::U) {
			std::string name = ((AST::UNode*)pattern->tail[i])->name;
			if (patternTemplateInputs.find(name) == patternTemplateInputs.end()) {
        if (throwExpIfNotFound) { 
          throw CompileDidNotUnderstandExpression();
        } else {
          pattern->tail[i] = nullptr;
          continue;
        }
      }
      //std::cout << "DICK\n";
     //printNode(patternTemplateInputs[name]);
      //std::cout << patternTemplateInputs[name]->type << '\n';
      AST::Node* inputCopy = AST::copyTree(patternTemplateInputs[name]);
			pattern->tail[i] = inputCopy;
      if (inputCopy != nullptr) inputCopy->head.push_back(pattern);
		} else {
			populatePattern(pattern->tail[i], patternTemplateInputs, throwExpIfNotFound);
		}
	}
}

/*
 * @brief Converts an expression to an equivalent simplified expression by directly replacing structures which are in the simplifaction table
 * @param head = The head of the AST of the expression
 * @return Returns the head of the simplified expression AST
*/
bool SlateContext::simplifyTree(SlateLanguage::AST::Node*& head) {
  
  bool modified = false;

	for (size_t i = 0; i < head->tail.size();i++) {
		AST::Node* simplified;
		while (simplifyTree(head->tail[i])) modified = true;
	}

	for (Simplification eq : definitions.simplifications) {
		std::unordered_map<std::string, SlateLanguage::AST::Node*> inputs;
		if (checkPatternHead(head, eq.from, inputs)) {
			AST::Node* pattern = AST::copyTree(eq.to);
			populatePattern(pattern, inputs);
      swapHeads(head,pattern); // TODO: memory
			head = pattern;
      modified = true;
      break;
		}
	}

	return modified;
}

void SlateContext::producePatternToTreeMoves(SlateLanguage::AST::Node* head, Property& p, std::vector<Move>& moves) {
  for (size_t i = 0;i < head->tail.size();i++) {
    if (head->tail[i] == nullptr) continue;
    std::vector<Move> tailPatterns;
    producePatternToTreeMoves(head->tail[i], p, tailPatterns);
    for (size_t j = 0;j < tailPatterns.size();j++) {
      AST::Node* ownCopy = AST::copyTree(head);
      ownCopy->tail[i] = tailPatterns[j].to;
      tailPatterns[j].to->head.push_back(ownCopy);
      moves.push_back({head, ownCopy, p});
    }
  } 

  std::unordered_map<std::string, AST::Node*> map;
  if(checkPatternHead(head, p.to, map)) {
    AST::Node* pattern = AST::copyTree(p.from);
    populatePattern(pattern, map);
    moves.push_back({head, pattern, p});
  }
  
}

void SlateContext::produceTreeToPatternMoves(SlateLanguage::AST::Node* head, Property& p, std::vector<Move>& moves, SlateLanguage::AST::Node* propertyHead) {
  if (propertyHead == nullptr) propertyHead = p.to;

  for (size_t i = 0;i < propertyHead->tail.size();i++) {
    produceTreeToPatternMoves(head, p, moves, propertyHead->tail[i]);
  }

  std::unordered_map<std::string, AST::Node*> map;
  if (checkPatternHead(head, propertyHead, map)) {
    AST::Node* pattern = AST::copyTree(p.from);
    populatePattern(pattern, map, false);
    moves.push_back({head, pattern, p});
  }

}

void SlateContext::produceMoves(AST::Node* head, Property& p, std::vector<Move>& moves) {
  producePatternToTreeMoves(head, p, moves);
  produceTreeToPatternMoves(head, p, moves);
  //std::cout << moves.size() << '\n';
}

bool inHistory(AST::Node* n, std::vector<AST::Node*>& h) {
  for (size_t i = 0;i < h.size();i++) {
    if (h[i] == n) return true;
  }
  return false;
}

int fromIndex(AST::Node* tail, AST::Node* head) {
  for (size_t i = 0;i < head->tail.size();i++) {
    if (tail == head->tail[i]) return i;
  }
  return -1;
}

// second patter to check
int SlateContext::localScoreNew(SlateLanguage::AST::Node* first, SlateLanguage::AST::Node* second, std::vector<SlateLanguage::AST::Node*> history) {
  if (second == nullptr) return 1;
  int score = 0;
  history.push_back(first);
  if (equalsNodes(first,second)) {
    score++;
    for (size_t i = 0;i < first->tail.size();i++) {
      if (second->tail.size() > i && !inHistory(first->tail[i], history)) score += localScoreNew(first->tail[i], second->tail[i],history);
    }
    for (size_t i = 0;i < first->head.size();i++) {
      if (second->head.size() > i && !inHistory(first->head[i], history)) {
        if (fromIndex(first, first->head[i]) == fromIndex(second, second->head[i])) {
          score += localScoreNew(first->head[i], second->head[i],history);
        }
      } 
    }
  }
  return score;
}

int SlateContext::globalScoreNew(SlateLanguage::AST::Node* first, SlateLanguage::AST::Node* second) {
  std::vector<AST::Node*> leafFirst = first->leafs();
  std::vector<AST::Node*> leafSecond = second->leafs();
  int scoreMax = INT_MIN;
  // TODO: bruh O(n^2)
  for (size_t i = 0;i < leafFirst.size();i++) {
    for (size_t j = 0;j < leafSecond.size();j++) {
      int score = localScoreNew(leafFirst[i], leafSecond[j]);
      if (score > scoreMax) scoreMax = score;
    }
  }
  return scoreMax;
}

int SlateContext::flaggedTailScore(SlateLanguage::AST::Node* first, SlateLanguage::AST::Node* second) {
  if (first == nullptr || second == nullptr) return -1;
  if (!first->getFlag(TREE_PIVOT_FLAG)) {
    for (size_t i = 0;i < first->tail.size();i++) {
      int res = flaggedTailScore(first->tail[i], second);
      if (res != -1) return res; 
    }
    return -1;
  } else {
    if (!second->getFlag(TREE_PIVOT_FLAG)) {
      for (size_t i = 0;i < second->tail.size();i++) {
        int res = flaggedTailScore(first, second->tail[i]);
        if (res != -1) return res;
      }
      return -1;
    } else {
      return localScoreNew(first, second);
    }
  }
}

// easy but bad
int SlateContext::markLastNode(SlateLanguage::AST::Node* tree, int depth, SlateLanguage::AST::Node* equals) {
  if (tree == nullptr) return -1; 
  tree->setFlags(TREE_PIVOT_FLAG, true);
  int maxDepth = depth;
  for (size_t i = 0;i < tree->tail.size();i++) {
    if (tree->tail[i] == nullptr) continue;
    int d = markLastNode(tree->tail[i],depth + 1, equals);
    if (d >= maxDepth) {
      maxDepth = d;
      tree->setFlags(TREE_PIVOT_FLAG, false);
      for (size_t j = 0;j < tree->tail.size();j++) {
        if (i != j && tree->tail[j] != nullptr) tree->tail[j]->clearFlags(); 
      }
    } else {
      tree->tail[i]->clearFlags();
    }
    //if (d == -1) tree->tail[i]->clearFlags();
  }
  if (maxDepth == depth) if (equals != nullptr && !equalsNodes(tree, equals)) {
    return -1;
  } 
  return maxDepth;
}

SlateLanguage::AST::Node* SlateContext::getAnchor(SlateLanguage::AST::Node* tree) {
  if (tree->getFlag(TREE_PIVOT_FLAG)) return tree;
  for (size_t i = 0;i < tree->tail.size();i++) {
    if (tree->tail[i] == nullptr) continue;
    AST::Node* returned = getAnchor(tree->tail[i]);
    if (returned != nullptr) return returned;
  }
  return nullptr;
}

// WE LOVE PARAMETERS I WANT MORE PARAMETERS
void SlateContext::generateSimplifcationTargets(RepetitionMap& head,
                                                RepetitionMap& pattern,
                                                size_t headSize,
                                                SlateLanguage::AST::Node* patternHead,
                                                std::vector<SlateLanguage::AST::Node*>& results,
                                                std::unordered_map<std::string, SlateLanguage::AST::Node*> mappings,
                                                size_t searchIndex) {
  if (searchIndex >= pattern.section.size()) {
    patternHead = AST::copyTree(patternHead);
    populatePattern(patternHead, mappings,false);
    if (!patternHead->anyNull() && patternHead->size() <= headSize) results.push_back(patternHead);
    return;
  }
  while (pattern.section[searchIndex]->type != SlateLanguage::AST::NodeTypes::U) {
     searchIndex++;
     if (searchIndex >= pattern.section.size()) {
        patternHead = AST::copyTree(patternHead);
        populatePattern(patternHead, mappings,false);
        if (!patternHead->anyNull() && patternHead->size() <= headSize) results.push_back(patternHead);
        return;
     }
  } 
  for (size_t i = 0;i < head.section.size();i++) {
    if (head.section[i] != nullptr && head.count[i] >= pattern.count[searchIndex]) {
      //std::cout << "F " << ((AST::UNode*)pattern.section[searchIndex])->name << " " << head.section[i]->type << " " << i << '\n';
      mappings[((AST::UNode*)pattern.section[searchIndex])->name] = head.section[i];
      head.count[i] -= pattern.count[searchIndex];
      generateSimplifcationTargets(head, pattern, headSize, patternHead, results, mappings, searchIndex + 1);
      head.count[i] += pattern.count[searchIndex];
    }
  } 
}

int SlateContext::stateScoreNewNew(SlateLanguage::AST::Node* first, SlateLanguage::AST::Node* second) {
  markLastNode(second);
  //std::cout << "FIRST\n";
  //printNode(second);
  AST::Node* pivot = getAnchor(second);
  markLastNode(first,0,pivot);
  //std::cout << "SECOND\n";
  //printNode(first);
  int score = flaggedTailScore(first, second);
  first->clearFlags();
  second->clearFlags();
  return score;
}

std::vector<Move>* SlateContext::AStartMoveSearch(SlateLanguage::AST::Node* start, SlateLanguage::AST::Node* end, std::vector<AST::Node*>& history) {
  start->clearFlags(); 
  //std::cout << " >>>>> SEARCH START WITH\n";
  //printNode(start);

  history.push_back(start);
  std::vector<Move> moves;
  for (Property& p : definitions.properties) {
     produceMoves(start, p, moves);
  } 
  std::vector<MoveResult> results;
  //std::cout << "PRODUCED MOVES\n";
  for (size_t i = 0;i < moves.size();i++) {
      bool loop = false;
      for (size_t j = 0;j < history.size();j++) {
        if (equalsAST(moves[i].to, history[j])) {
          loop = true;
          break;
        }
      }
      if (loop) results.push_back({moves[i],0});
      else {
        int score  = stateScoreNewNew(end,moves[i].to);
        results.push_back({moves[i], score});
      }
    //printNode(moves[i]);
    //std::cout << "SCORE: " << scores[i] << '\n';
    //std::cout << "SIZE: " << moves[i]->size();
  }

  //int maxScore = INT_MIN;
  //for (size_t i = 0;i < moves.size();i++) {
    //if (scores[i] > maxScore) maxScore = scores[i];
  //}
  //std::cout << "MAX SCORE: " << maxScore << '\n';

  std::sort(results.begin(), results.end(), [](MoveResult a, MoveResult b) {
    return a.score > b.score;
  });

  for (size_t i = 0;i < results.size();i++) {
    if (true) { // TODO: ||true
      AST::Node* best = results[i].move.to;
      //std::cout << "TEST\n";
      //printNode(best);
      if (best == nullptr || best->size() > end->size()) {
        continue;
      }
      bool loop = false;
      for (size_t j = 0;j < history.size();j++) {
        if (equalsAST(best, history[j])) {
          loop = true;
          break;
        }
      }
      if (loop) continue;
      //if (equalsAST(best,start)) {
      //  continue;
      //}
      //std::cout << "NEXT MOVE\n";
      //printNode(best);
      //std::cout << "SCORE: " << scores[i] << '\n';
      if (results[i].score == best->size()) {
        std::cout << "SCORE REACHED\n";
        printNode(best);
        std::vector<Move>* movesHistory = new std::vector<Move>();
        movesHistory->push_back(results[i].move);
        return movesHistory;
      } 
      std::vector<Move>* moveHistory = AStartMoveSearch(best, end, history);
      if(moveHistory != nullptr) {
        moveHistory->push_back(results[i].move);
        return moveHistory;
      }
    }
  }

  std::cout << ":(\n";
  return nullptr;
}

bool SlateContext::containsTree(AST::Node* fixed, AST::Node* other) {
  if (equalsAST(fixed, other, true)) return true;
  for (size_t i = 0;i < fixed->tail.size();i++) {
    if (containsTree(fixed->tail[i], other)) return true;
  }
  return false;
}

void SlateContext::invertMove(SlateLanguage::AST::Node*& head, AST::Node*& originalHead, Move& m) {
  std::unordered_map<std::string, AST::Node*> inputs;
  if (containsTree(AST::treeTop(head), m.from)) return;
  std::cout << "MMM\n";
  printNode(head);
  printNode(m.propertyUsed.from);
  printNode(m.from);
  if (checkPatternHead(head, m.propertyUsed.from, inputs)) {
    std::cout << inputs.size() << "PATTERN FOUND\n";
    AST::Node* copy = AST::copyTree(m.propertyUsed.to);
    populatePattern(copy, inputs);
    std::cout << "FSDF\n";
    AST::Node* headCopy = AST::copyTreeWhole(head);
    std::cout << "HEAD COPY\n";
    swapHeads(headCopy, copy);
    printNode(AST::treeTop(copy));
    if (containsTree(AST::treeTop(copy), m.from)) {
      std::cout << "YAY\n";
      swapHeads(head, copy);
      head = copy;
      return;
    }
  }
  for (size_t i = 0;i < head->tail.size();i++) {
    if (containsTree(originalHead, m.from)) break;
    invertMove(head->tail[i], originalHead, m);
  }
}

SlateLanguage::AST::Node* SlateContext::reduceTree_(SlateLanguage::AST::Node* head) {
  do {
    for (size_t i = 0;i < definitions.simplifications.size();i++) {
      RepetitionMap headMap;
      RepetitionMap simplificationMap;
      bool mayDo = heuristicDoSimplification(head, definitions.simplifications[i], headMap, simplificationMap);
      if (mayDo) {
        std::vector<AST::Node*> starts;
        generateSimplifcationTargets(headMap, simplificationMap, head->size(), definitions.simplifications[i].from, starts);
        for (size_t j = 0;j < starts.size();j++) {
          std::vector<AST::Node*> h;
          std::vector<Move>* moves;
          if ((moves = AStartMoveSearch(starts[j], head, h))) {
            std::cout << "FINAL IS\n";
            AST::Node* c = AST::copyTree(head);
            for (size_t m = 0;m < moves->size();m++) invertMove(c,c,(*moves)[m]);
            printNode(c);
            return c;
          }
        }
      }
    }
  } while(false && simplifyTree(head));
  return head;
}
