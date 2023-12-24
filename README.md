# KariLang

Toy Functional Programming Language

## Example Program

```text
valdef int x = 10

funcdef fib n int -> int = 
    if n < 2 then 
        n 
    else 
        fib(n + -1) + fib(n + -2)

funcdef main n int -> int = fib(n)
```

To Run

```bash
KariLang ./program.txt 15
```

## About the language

It has only 2 data types, `int` and `bool`.
The language does not have any looping structures,
but repeated code execution can be achieved using recursion.

There is no meaning full error message at all.
So, if your program does not execute it is due to some syntax or semantic error.
Sorry, that there is no error message.

In future, if I am free, I would like to add `match ... with ...` statement
along with support of algebraic data types.
Also, add support for higher order functions.

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
cc -Wall -g ./main.c ./semantics.c ./interpreter.c ./lex.yy.c ./parser.tab.c -o ./KariLang
```
