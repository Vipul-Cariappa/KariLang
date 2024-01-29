BUILD_DIR=bin
SRC_DIR=src

LEXER_FILE = $(SRC_DIR)/Lexer.l
LEXER_OUTPUT = $(BUILD_DIR)/Lexer.yy.cc

PARSER_FILE = $(SRC_DIR)/Parser.yy
PARSER_OUTPUT = $(BUILD_DIR)/Parser.tab.cc

SOURCE_FILES = $(SRC_DIR)/Main.cc

# TODO: use the below variable
INCLUDE_DIR = \
  $(SRC_DIR) \
  $(BUILD_DIR)

C = clang
CXX = clang++
BUILD_OPTIONS = -g -fsanitize=address -fno-omit-frame-pointer

$(BUILD_DIR)/KariLang: $(SOURCE_FILES) $(LEXER_OUTPUT) $(PARSER_OUTPUT)
	$(CXX) $(BUILD_OPTIONS) $^ -o $@

$(LEXER_OUTPUT): $(LEXER_FILE) $(PARSER_OUTPUT)
	flex -o $@ $(LEXER_FILE)

$(PARSER_OUTPUT): $(PARSER_FILE)
	bison -H $^ -o $@

.PHONY: clean
clean:
	rm bin/*
