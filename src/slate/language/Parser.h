#pragma once
#include "Lexer.h"
#include "AST.h"

namespace SlateLanguage {

	namespace Parser {

		extern AST::Node* parser(std::vector<SlateLanguage::Lexer::Token*>& wrappers);
	}
}