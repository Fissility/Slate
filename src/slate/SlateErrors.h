#pragma once

#define OK {0,"OK",0,0}

// Lexer

#define OUT_OF_PLACE(S,E) {-1, std::format("Out of place symbol at {}",S),S,E}
#define EMPTY_SUBSCRIPT(S,E) { -2, std::format("Empty subscript at {}",S),S,E}
#define UNCLOSED_SUBSCRIPT(S,E) { -3, std::format("Unclosed subscript at {}",S),S,E}
#define EMPTY_FLARE(S,E) {-4, std::format("Empty flare at {}",S),S,E}
#define UNCLOSED_FLARE(S,E) {-5, std::format("Unclosed flare at {}",S),S,E}
#define UNCLOSED_FRACTION(S,E) {-6, std::format("Unclosed fraction at {}",S),S,E}

// Parser

#define BRACKET_NOT_CLOSED(S,E) {-6, std::format("Bracket at {} was not closed",i),S),S,E}
#define BRACKET_NOT_OPENED(S,E) {-7, std::format("Bracket at {} was nenver opened",S),S,E}
#define OPERATOR_NOT_DEFINED(S,E) {-8, std::format("Operator at {} was never defined",E),S,E}
#define FLOATING_OPERATOR(S,E) {-9, std::format("Operator doesn't have elements on both sides"),S,E}