#include "SlateContext.h"
#include "SlateDefinitions.h"

#include "language/AST.h"
#include "language/SlateErrors.h"
#include "language/Tokenizer.h"
#include "language/Lexer.h"
#include "language/Parser.h"
#include "objects/Expression.h"
#include "objects/tuple/BiCategory.h" //X
#include <iostream>
#include <iterator>
#include <stack>
#include "SlateType.h"

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

void SlateContext::getRepetitionSignature(AST::Node* head, std::vector<AST::Node*>& subSection, std::vector<size_t>& sameCount) {
  for (size_t i = 0;i < head->tail.size();i++) {
    getRepetitionSignature(head->tail[i],subSection,sameCount);
  }

  for (size_t i = 0;i < subSection.size();i++) {
    if (equalsAST(head,subSection[i])) {
      sameCount[i]++;
      for(size_t j = 0;j < i;j++) {
        if (sameCount[j] < sameCount[i]) {
          size_t temp = sameCount[j];
          sameCount[j] = sameCount[i];
          sameCount[i] = temp;
          AST::Node* tempNode = subSection[j];
          subSection[j] = subSection[i];
          subSection[i] = tempNode;
          break;
        }
      }
      return;
    }
  }

  subSection.push_back(head);
  sameCount.push_back(1);
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

bool SlateContext::maybeDoSimplification(SlateLanguage::AST::Node* head, Simplification& s) {
  if (!includesAllBlocks(head,s.from)) return false;
  std::vector<AST::Node*> sectionsTemp;
  std::vector<size_t> signatureHead;
  std::vector<size_t> signatureS;
  getRepetitionSignature(head, sectionsTemp, signatureHead);
  sectionsTemp.clear();
  getRepetitionSignature(s.from, sectionsTemp, signatureS);
  if (!doSignaturesMatch(signatureHead, signatureS)) return false;
  return true;
} 

/*
 * @brief Processes and updates the context with the TeX expression offered in the inputted string.
 * @param line = The string whicb holds the TeX expression
 * @return Returns the relevant objects in relation to the inputted expression. The pointer can also be null if there is no relevant output (e.g. f:A->B doesn't have any relevant output)
 */
Object* SlateContext::interpret(std::string line) {

	std::vector<Lexer::Token*> tokens;
	Lexer::lexer(line, definitions, tokens);

	AST::Node* output = Parser::parser(tokens);
	AST::Node* outputCopy = AST::copyTree(output);

	printNode(output);
	std::cout << "\n";
  std::vector<AST::Node*> prv;
  if (reduceTree(outputCopy, outputCopy, prv)) {
    printNode(outputCopy);
    std::vector<std::string> u;
    std::cout << displayStringFromAST(outputCopy) << '\n';
  } else {
    std::cout << "Could not be simplified!\n";
  }

  for (Simplification& s : definitions.simplifications) {
    std::cout << maybeDoSimplification(output,s) << '\n';
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
bool SlateContext::equalsNodes(SlateLanguage::AST::Node* first, SlateLanguage::AST::Node* second) {
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

bool SlateContext::equalsAST(SlateLanguage::AST::Node* first, SlateLanguage::AST::Node* second) {
	if (!equalsNodes(first, second)) return false;
	for (size_t i = 0; i < first->tail.size(); i++) {
		if (!equalsAST(first->tail[i], second->tail[i])) return false;
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

void SlateContext::populatePattern(SlateLanguage::AST::Node*& pattern, std::unordered_map<std::string, SlateLanguage::AST::Node*>& patternTemplateInputs) {

  if (pattern->type == AST::NodeTypes::U) {
    // TODO: free the existing pattern
    pattern = AST::copyTree(patternTemplateInputs[((AST::UNode*)pattern)->name]);
    return;
  }

  for (size_t i = 0; i < pattern->tail.size();i++) {
		if (pattern->tail[i]->type == AST::NodeTypes::U) {
			std::string name = ((AST::UNode*)pattern->tail[i])->name;
			if (patternTemplateInputs.find(name) == patternTemplateInputs.end()) throw CompileDidNotUnderstandExpression();
      AST::Node* inputCopy = AST::copyTree(patternTemplateInputs[name]);
			pattern->tail[i] = inputCopy;
      inputCopy->head.push_back(pattern);
		} else {
			populatePattern(pattern->tail[i], patternTemplateInputs);
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
      swapHeads(head,pattern);
			head = pattern;
      modified = true;
      break;
		}
	}

	return modified;
}
size_t l = 0;
// The looping works since it is a finite group so elements have finite order
bool SlateContext::reduceTree(SlateLanguage::AST::Node*& top, SlateLanguage::AST::Node*& head, std::vector<AST::Node*>& previous) {
  //std::cout << '\n';
  //printNode(top);

  for (Property pr : definitions.properties) {
    std::unordered_map<std::string, AST::Node*> inputs;
    if (checkPatternHead(head, pr.from, inputs)) { 

      AST::Node* pattern = AST::copyTree(pr.to);
      populatePattern(pattern, inputs);
      
      AST::Node* old = head;
      swapHeads(head,pattern);
      head = pattern; 

      bool looping = false;
      for (AST::Node* back : previous) {
        if (equalsAST(top,back)) {
          looping = true;
          break;
        }
      }
      
      if (looping) {
        swapHeads(head,old);
        head = old;
        continue;
      }

      previous.push_back(AST::copyTree(top));


      if (simplifyTree(top)) {
        std::cout << "BEEP\n";
        std::vector<AST::Node*> prv;
        reduceTree(top,top,prv);
        return true;
      }

      if(reduceTree(top, head, previous)) {
        std::cout << "BOOP\n";
        return true;
      }
      

      bool anyTailsReduced = false;

      for (size_t i = 0;i < head->tail.size();i++) {
        std::vector<AST::Node*> prv;
        //std::cout << l++ << '\n';
        if (reduceTree(top,head->tail[i],prv)) {
          anyTailsReduced = true;
        }
        //std::cout << l-- << '\n';
      }

      if (anyTailsReduced) {
        simplifyTree(pattern);
        return true;
      }

      swapHeads(head,old);
      head = old;
      previous.pop_back();
      
    }
  } 
  
  return false;
}
