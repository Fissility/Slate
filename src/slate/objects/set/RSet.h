#pragma once

#include "QSet.h"

// There is no difference between R and Q when using numerical methods
class RSet : public QSet {
public:

	std::string toString() {
		return "\\mathbb{R}";
	}
};