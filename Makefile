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

INCLUDE_OPTIONS = \
  -I$(SRC_DIR) \
  -I$(BUILD_DIR)

C = clang
CXX = clang++
BUILD_OPTIONS = \
  -Wall \
  -g \
  `llvm-config --cxxflags --ldflags --system-libs --libs all`
  # -fsanitize=address \
  # -fno-omit-frame-pointer

$(BUILD_DIR)/KariLang: $(SOURCE_FILES) $(LEXER_OUTPUT) $(PARSER_OUTPUT) $(HEADER_FILES)
	$(CXX) $(INCLUDE_OPTIONS) $(SOURCE_FILES) $(LEXER_OUTPUT) $(PARSER_OUTPUT) $(BUILD_OPTIONS) -o $@

$(LEXER_OUTPUT): $(LEXER_FILE) $(PARSER_OUTPUT)
	flex -o $@ $(LEXER_FILE)

$(PARSER_OUTPUT): $(PARSER_FILE)
	bison -H $^ -o $@

.PHONY: clean
clean:
	rm bin/* && rm tmp/*.o
