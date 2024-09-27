#pragma once

#define OK {0,"OK"}

// Lexer

#define OUT_OF_PLACE {-1, std::format("Out of place symbol at {}",i)}
#define EMPTY_SUBSCRIPT { -2, std::format("Empty subscript at {}",i) }
#define UNCLOSED_SUBSCRIPT { -3, std::format("Unclosed subscript at {}",i) }
#define EMPTY_FLARE {-4, std::format("Empty flare at {}",i)}
#define UNCLOSED_FLARE {-5, std::format("Unclosed flare at {}",i)}
#define UNCLOSED_FRACTION {-6, std::format("Unclosed fraction at {}",i)}

// Parser