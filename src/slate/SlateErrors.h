#pragma once

#define OK {0,"OK"}
#define ABRUPT_ENDING {-1,"Expression ended abruptly"}
#define EMPTY_SUBSCRIPT { -2, std::format("Empty subscript at {}",i) }
#define UNCLOSED_SUBSCRIPT { -3, std::format("Unclosed subscript at {}",i) }
#define EMPTY_FLARE {-4, std::format("Empty flare at {}",i)}
#define UNCLOSED_FLARE {-5, std::format("Unclosed flare at {}",i)}