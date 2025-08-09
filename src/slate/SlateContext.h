#pragma once

#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include "language/AST.h"
#include "objects/Expression.h"
#include "language/Parser.h"
#include "objects/Object.h"
#include "SlateDefinitions.h"

struct PivotTree {
  SlateLanguage::AST::Node* head;
  SlateLanguage::AST::Node* pivot;
};

struct RepetitionMap {
  std::vector<SlateLanguage::AST::Node*> section;
  std::vector<size_t> count;

  void join(RepetitionMap other) {
    for (size_t i = 0;i < other.section.size();i++) {
      section.push_back(other.section[i]);
      count.push_back(other.count[i]);
    }
  }
};

struct Move {
  SlateLanguage::AST::Node* from;
  SlateLanguage::AST::Node* to;
  Property& propertyUsed;
  Move& operator=(const Move& other) {
    if (this != &other) {
      from = other.from;
      to = other.to;
      propertyUsed = other.propertyUsed;
    }
    return *this;
  }
};

struct MoveResult {
  Move move;
  int score;
  MoveResult& operator=(const MoveResult& other) {
    if (this != &other) {
      move = other.move;
      score = other.score;
    }
    return *this;
  }
};

class SlateContext {
private:

	std::string displayStringFromAST(SlateLanguage::AST::Node* head);
	Expression* expressionFromAST(SlateLanguage::AST::Node* head, std::vector<std::string>* unknowns);

  std::unordered_map<SlateLanguage::AST::Node*,std::vector<size_t>> simplificationSignatures;

  bool doSignaturesMatch(std::vector<size_t>& expression, std::vector<size_t>& simplification); // TODO: Implement it
  bool includesNode(SlateLanguage::AST::Node* node, SlateLanguage::AST::Node* pattern);
  bool includesAllBlocks(SlateLanguage::AST::Node* head, SlateLanguage::AST::Node* pattern);
  bool heuristicDoSimplification(SlateLanguage::AST::Node* head, Simplification& s, RepetitionMap& rHead, RepetitionMap& rSimp); // TODO: Implement it
  void generateSimplifcationTargets(RepetitionMap& head,
                                    RepetitionMap& pattern,
                                    size_t headSize,
                                    SlateLanguage::AST::Node* patternHead,
                                    std::vector<SlateLanguage::AST::Node*>& results,
                                    std::unordered_map<std::string, SlateLanguage::AST::Node*> mappings = std::unordered_map<std::string, SlateLanguage::AST::Node*>(),
                                    size_t searchIndex = 0);
  std::vector<Move>* AStartMoveSearch(SlateLanguage::AST::Node* start, SlateLanguage::AST::Node* end, std::vector<SlateLanguage::AST::Node*>& history);

public:

	Definitions definitions;

	std::string getObjectName(Object* o);

  void addDefinitions(Definitions& defs);
  SlateLanguage::AST::Node* toAST(std::string line);
	Object* interpret(std::string line);

	void printNode(SlateLanguage::AST::Node* n, size_t spaces = 0);

	bool equalsNodes(SlateLanguage::AST::Node* first, SlateLanguage::AST::Node* second, bool nullPlaceholder = false);
	bool equalsAST(SlateLanguage::AST::Node* first, SlateLanguage::AST::Node* second, bool nullPlaceholder = false);
  void swapHeads(SlateLanguage::AST::Node* target, SlateLanguage::AST::Node* source);
	bool checkPatternHead(SlateLanguage::AST::Node* head, SlateLanguage::AST::Node* inputPattern, std::unordered_map<std::string,SlateLanguage::AST::Node*>& patternTemplateInputs);
  void populatePattern(SlateLanguage::AST::Node*& replacement, std::unordered_map<std::string, SlateLanguage::AST::Node*>& patternTemplateInputs, bool throwExpIfNotFound = true);
  bool simplifyTree(SlateLanguage::AST::Node*& head);
  void getRepetitionSignature(SlateLanguage::AST::Node* head, RepetitionMap& map); 

  void producePatternToTreeMoves(SlateLanguage::AST::Node* head, Property& p, std::vector<Move>& moves);
  void produceTreeToPatternMoves(SlateLanguage::AST::Node* head, Property& p, std::vector<Move>& moves, SlateLanguage::AST::Node* propertyHead = nullptr);
  void produceMoves(SlateLanguage::AST::Node* head, Property& p, std::vector<Move>& moves);

  int localScoreNew(SlateLanguage::AST::Node* first, SlateLanguage::AST::Node* second, std::vector<SlateLanguage::AST::Node*> history = std::vector<SlateLanguage::AST::Node*>());
  int globalScoreNew(SlateLanguage::AST::Node* first, SlateLanguage::AST::Node* second);
  int flaggedTailScore(SlateLanguage::AST::Node* first, SlateLanguage::AST::Node* second);
  int stateScoreNewNew(SlateLanguage::AST::Node* first, SlateLanguage::AST::Node* second);

  bool containsTree(SlateLanguage::AST::Node* fixed, SlateLanguage::AST::Node* other);
  int markLastNode(SlateLanguage::AST::Node* tree, int depth = 0, SlateLanguage::AST::Node* equals = nullptr);
  SlateLanguage::AST::Node* getAnchor(SlateLanguage::AST::Node* tree); 

  void invertMove(SlateLanguage::AST::Node*& head, SlateLanguage::AST::Node*& originalHead, Move& m);
  SlateLanguage::AST::Node* reduceTree_(SlateLanguage::AST::Node* head);
};
