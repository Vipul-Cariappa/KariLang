# KariLang

Toy Functional Programming Language

You can test out the language using Jupyter Notebook [here](https://www.vipulcariappa.xyz/jupyterlite-deploy/lab/index.html?path=xeus-karilang.ipynb).

## Example Program

```text
valdef zero: int = 0;
valdef one: int = 1;
valdef two: int = one + one;

funcdef sum(n: int) -> int = _sum(zero, n);

funcdef _sum(c: int, n: int) -> int =
    if n == zero then
        c
    else
        _sum(c + n, n + -1);

funcdef fib(n: int) -> int = 
    if n < two then
        n 
    else
        fib(n + -1) + fib(n + -two);

funcdef main(n: int) -> int = fib(n);
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

You can download the compiled binary for Windows and Linux directly 
from the [releases](https://github.com/Vipul-Cariappa/KariLang/releases) page.

If you want to compile from source:

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
