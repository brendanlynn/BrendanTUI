-include local_environment.mk

# Project name
PROJECT_NAME = brendantui

# Compiler and flags
CXX = cl
CXXFLAGS = /std:c++20 /O2 /W4 /I include /EHsc
LDFLAGS =

# Libraries
LIBS =

# Directories
SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = obj
OBJ_SRC_DIR = $(OBJ_DIR)/src
OBJ_TESTS_DIR = $(OBJ_DIR)/tests
BIN_DIR = bin
TESTS_DIR = tests

# Source files
CPP_SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
TEST_SOURCES = $(wildcard $(TESTS_DIR)/*.cpp)

# Object files
CPP_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_SRC_DIR)/%.obj, $(CPP_SOURCES))
TEST_OBJECTS = $(patsubst $(TESTS_DIR)/%.cpp, $(OBJ_TESTS_DIR)/%.obj, $(TEST_SOURCES))

# Library output
STATIC_LIB = $(BIN_DIR)/$(PROJECT_NAME).lib
TEST_EXECUTABLE = $(BIN_DIR)/$(PROJECT_NAME)_tests.exe

# Default target
all: $(if $(CPP_SOURCES), $(STATIC_LIB),) $(if $(TEST_SOURCES), $(TEST_EXECUTABLE),)

# Compile C++ source files
$(OBJ_SRC_DIR)/%.obj: $(SRC_DIR)/%.cpp | $(OBJ_SRC_DIR)
	$(CXX) $(CXXFLAGS) /c $< /Fo$@

# Build static library (.lib)
$(STATIC_LIB):  $(CPP_OBJECTS) $(LIB_FILES) | $(BIN_DIR)
	lib /OUT:$@ $^

# Compile tests
$(OBJ_TESTS_DIR)/%.obj: $(TESTS_DIR)/%.cpp | $(OBJ_TESTS_DIR)
	$(CXX) $(CXXFLAGS) /c $< /Fo$@

# Link test executable (depends on static library)
ifneq ($(TEST_SOURCES),)
$(TEST_EXECUTABLE): $(TEST_OBJECTS) $(if $(CPP_SOURCES), $(STATIC_LIB),) $(LIB_FILES) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) /Fe$@ $(LDFLAGS)
endif

# Run tests
ifeq ($(TEST_SOURCES),)
check:
	@echo No tests found.
else
check: $(TEST_EXECUTABLE)
	$(TEST_EXECUTABLE)
endif

# Create output directories
$(OBJ_DIR):
	$(shell mkdir $(subst /,\,$(OBJ_DIR)))
$(OBJ_SRC_DIR):
	$(shell mkdir $(subst /,\,$(OBJ_SRC_DIR)))
$(OBJ_TESTS_DIR):
	$(shell mkdir $(subst /,\,$(OBJ_TESTS_DIR)))
$(BIN_DIR):
	$(shell mkdir $(subst /,\,$(BIN_DIR)))

# Clean build files
clean:
	if exist $(OBJ_DIR) rd /s /q $(OBJ_DIR);
	if exist $(BIN_DIR) rd /s /q $(BIN_DIR);

.PHONY: all clean check
