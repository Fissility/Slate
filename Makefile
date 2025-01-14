# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 --debug -Wall -Wextra

# Find all cpp files in the current directory and all subdirectories
SRC_FILES = $(shell find . -name "*.cpp")

# Find all object files corresponding to the cpp files
OBJ_FILES = $(SRC_FILES:.cpp=.o)

# The output executable name (you can change this as needed)
EXEC = my_program

# Default target: build the program
all: $(EXEC)

# Link the object files into the final executable
$(EXEC): $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) -o $(EXEC)

# Compile each .cpp file into a .o object file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up the generated object files and executable
clean:
	rm -f $(OBJ_FILES) $(EXEC)

# Phony targets (to prevent conflicts with filenames)
.PHONY: all clean
