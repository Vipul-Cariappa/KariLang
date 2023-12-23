# KariLang

Toy Functional Programming Language

## To Compile

Go to src directory
```bash
cd src/
```

Compile the parser
```bash
bison -Wall -Wcounterexamples -H ./parser.y
```

Compiler the lexer
```bash
flex ./lexer.l
```

Compiler the language
```bash
cc -Wall -g ./main.c ./lex.yy.c ./parser.tab.c
```
