#include "Types.h"

std::string typeNames[] = {
	"UNKNOWN",
	"NUMBER",
	"INTERVAL",
	"SET",
	"FUNCTION",
	"BINARY_FUNCTION",
	"TUPLE",
	"CATEGORY",
	"EXPRESSION"
};

std::string Types::getTypeName(Type type) {
	return typeNames[type];
}