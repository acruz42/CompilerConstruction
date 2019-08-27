# Adrian Cruz
# CS 370 - Lab 9

# makefile to create the lab9 executable

all:
	yacc -d lab9.y
	lex lab9.l
	gcc -c y.tab.c -w
	gcc -c lex.yy.c
	gcc -c ast.c
	gcc -c symtable.c
	gcc -c emit.c
	gcc y.tab.o lex.yy.o ast.o symtable.o emit.o -o lab9
	rm *.o
