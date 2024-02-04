BUILD_DIR=bin
SRC_DIR=src

LEXER_FILE = $(SRC_DIR)/Lexer.l
LEXER_OUTPUT = $(BUILD_DIR)/Lexer.yy.cc

PARSER_FILE = $(SRC_DIR)/Parser.yy
PARSER_OUTPUT = $(BUILD_DIR)/Parser.tab.cc

SOURCE_FILES = \
  $(SRC_DIR)/Main.cc \
  $(SRC_DIR)/AST.cc \
  $(SRC_DIR)/Compile.cc

HEADER_FILES = \
  $(SRC_DIR)/Utils.hh \
  $(SRC_DIR)/AST.hh

# TODO: use the below variable
INCLUDE_OPTIONS = \
  -I$(SRC_DIR) \
  -I$(BUILD_DIR)

C = clang
CXX = clang++
BUILD_OPTIONS = \
  -Wall \
  -g \
  -fsanitize=address \
  -fno-omit-frame-pointer \
  `llvm-config --cxxflags --ldflags --system-libs --libs all`

$(BUILD_DIR)/KariLang: $(SOURCE_FILES) $(LEXER_OUTPUT) $(PARSER_OUTPUT) $(HEADER_FILES)
	$(CXX) $(BUILD_OPTIONS) $(INCLUDE_OPTIONS) $(SOURCE_FILES) $(LEXER_OUTPUT) $(PARSER_OUTPUT) -o $@

$(LEXER_OUTPUT): $(LEXER_FILE) $(PARSER_OUTPUT)
	flex -o $@ $(LEXER_FILE)

$(PARSER_OUTPUT): $(PARSER_FILE)
	bison -H $^ -o $@

.PHONY: clean
clean:
	rm bin/*
