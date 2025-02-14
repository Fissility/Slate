#pragma once

#include "language/AST.h"
#include "objects/function/BinaryOperator.h"
#include "objects/function/Function.h"
#include "objects/set/BSet.h"
#include "objects/set/IntervalSet.h"
#include "objects/set/NSet.h"
#include "objects/set/QSet.h"
#include "objects/set/RSet.h"
#include "objects/set/USets.h"
#include "objects/set/ZSet.h"
#include <climits>
#include <string>
#include <vector>
#include <unordered_map>

enum Presidences {
  EQUALS = INT_MIN,
  ELEMENT_BINDING = 0,
  ADDITION,
  SUBTRACTION = ADDITION,
  MULTIPLICATION,
  DIVISION = MULTIPLICATION,
  POWER,
  SET_BINDING,
  CARTESIAN
};

// Simplifications when applied to an expression yields it closer to having every function node being inversible in one step.
struct Simplification {
  SlateLanguage::AST::Node* from;
  SlateLanguage::AST::Node* to;
  Simplification(SlateLanguage::AST::Node *from, SlateLanguage::AST::Node *to) {
    this->from = from;
    this->to = to;
  }
};

// Properties when applied to an expression do not change its complexity but merely shuffle elements around.
struct Property {
  SlateLanguage::AST::Node* from;
  SlateLanguage::AST::Node* to;
  Property(SlateLanguage::AST::Node* from, SlateLanguage::AST::Node* to) {
    this->from = from;
    this->to = to;
  }
};

struct Definitions {

  std::vector<Simplification> simplifications;
  std::vector<Property> properties;
  // The word definition here refers to the name - object assosciation
  // Map of objects and the names by which they are identified
  std::unordered_map<std::string, Object*> definitions;
  // List of all objects that have definitions
  std::vector<Object*> definedObjects;
  // Map of the printable strings of some objects (i.e. not all objects are
  // required to have a printable string stored)
  std::unordered_map<Object*, std::string> stringValues;

  /*
   * @brief Adds a new definition.
   * @param name = The name of the new object added
   * @param o = The new object
   */
  void registerDefinition(std::string &name, Object *o);
  /*
   * @param name = The name of the definition
   * @return Returns TRUE if the name is associated with a definition, FALSE
   * otherwise
   */
  bool definitionExists(std::string &name);
  /*
   * @param name = The object
   * @return Returns TRUE if there is a definition for the inputted object,
   * FALSE otherwise
   */
  bool objectHasDefinition(Object *o);
  /*
   * @param name = The name of the definition
   * @return Returns the object assosciated with the definition. Returns nullptr
   * if the definition doesn't exist.
   */
  Object *getDefinition(std::string &name);

  /*
   * @brief Registers a display string mapping for an object
   */
  void registerString(Object *o, std::string string);
  /*
   * @param o = The object for which it should check if a name mapping exists
   * @return Returns TRUE if the object has a display name, FALSE otherwise
   */
  bool objectHasString(Object *o);
  /*
   * @param o = The object for which the display name should be returned
   * @return Returns the display name for the inputted object. Returns "[Object
   * cannot be displayed]" if it does not have a string registered.
   */
  std::string getString(Object *o);

  /*
   * @brief Registeres an object for which its definition name and display
   * string are the same
   * @param o = The object which should be registered
   * @param name = The name of the object
   */
  void registerBaseObject(Object *o, std::string name);

  void registerSimplification(Simplification eq);
  void registerProperty(Property pr);

  /*
   * @brief Clears all definitions and display strings
   */
  void clear();

  /*
   * @brief Adds the definitions and display strings from other set of
   * definitions to this one. If there are duplicates of any kind then the
   * entries are not modified.
   */
  void addFrom(Definitions other) {
    definitions.insert(other.definitions.begin(), other.definitions.end());
    stringValues.insert(other.stringValues.begin(), other.stringValues.end());
    definedObjects.insert(definedObjects.begin(), other.definedObjects.begin(),other.definedObjects.end());
    simplifications.insert(simplifications.begin(), other.simplifications.begin(),other.simplifications.end());
    properties.insert(properties.begin(), other.properties.begin(), other.properties.end());
  }
};

// Holds the set of symbols and default definitions for initialising a context
namespace SlateDefinitions {

extern std::vector<std::string> symbolBases;
extern std::vector<std::string> symbolFlares;
extern std::vector<std::string> controlSeqeuenceCharacters;
extern std::vector<std::string> binaryOperators;
extern std::unordered_map<std::string, std::string> controlSequenceFunctions;

extern bool equals(Object *first, Object *second);

extern bool objectIsIn(Object *o, Set *s);
/*
 * @brief The union operation on two sets
 * @return Returns a set which is the union of the two
 */
extern Set *setUnion(Set *first, Set *second);
/*
 * @brief Intersection operation on two sets
 * @return Returns a set which is the intersection of the two
 */
extern Set *setIntersection(Set *first, Set *second);
/*
 * @brief Minus operation between two sets
 * @return Returns a set which contains all elements in the first set that are
 * not in the second set
 */
extern Set *setMinus(Set *first, Set *second);
/*
 * @brief Cartesian operation between two sets
 * @return Returns the set which is the cartesian product of the two sets
 */
extern Set *setCartesian(Set *first, Set *second);

extern void loadSymbols();
extern Definitions buildDefaultDefinitions();

} // namespace SlateDefinitions
