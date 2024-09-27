CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++20 -I$(SRC_DIR) # Include src as header path

# Directories
SRC_DIR = slate
OBJ_DIR = obj
OBJS_DIRS = $(OBJ_DIR)/objects $(OBJ_DIR)/objects/function $(OBJ_DIR)/objects/number $(OBJ_DIR)/objects/set $(OBJ_DIR)/objects/tuple

# Find all .cpp files recursively in src
SRCS = $(shell find $(SRC_DIR) -name "*.cpp")

# Convert .cpp sources into .o object files in obj/
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Target executable
TARGET = slate_executable

# Default rule to build the target
all: $(TARGET)

# Rule to link object files into the final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule to compile .cpp files to .o files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@mkdir -p $(OBJS_DIRS) # Ensure the directories exist
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to create obj directory
$(OBJ_DIR):
	mkdir -p $(OBJS_DIRS)

# Clean rule to remove compiled objects and the target
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Phony targets to avoid name conflicts
.PHONY: all clean