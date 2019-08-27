/*

Adrian Cruz
CS 370 - Lab 9

Abstract Syntax Tree header file

This program lists the enums for the ASTtypes and the operators that can be
used when running the program. As well as the ASTnode struct.

Added the SYSTYPE enum for the type of variable, 'int' 'void' 'boolean'

*/

#include<stdio.h>
#include<malloc.h>

#ifndef AST_H
#define AST_H

static int mydebug1 = 0;

/* define the enumerated types for the AST.  THis is used to tell us what
   sort of production rule we came across */

enum ASTtype {
   BLOCK,
   VARDEC,
   FUNDEC,
   PARAM,
   READSTMT,
   IDENT,
   WRITESTMT,
   CALL,
   NUMBER,
   RETURNSTMT,
   WHILESTMT,
   ASSIGNSTMT,
   EXPR,
   IFSTMT,
   THENSTMT,
   EMPTYSTMT,
   ARGLIST,
};

/* define the enumerated types for the operators of the AST. This is used
   to tell us which kind of operation are the expressions */
enum OPERATORS {
   PLUS,
   MINUS,
   TIMES,
   LESSTHAN,
   GREATERTHAN,
   EQUALTO,
   NOTEQUAL,
   LESSTHANEQ,
   GREATERTHANEQ,
   DIVIDE,
   MYAND,
   MYOR,
   MYNOT,
   TRU,
   FAL
};

/* new enum to separate the types of the operators */
enum SYSTYPE {
   BOOLTYPE,
   VOIDTYPE,
   INTTYPE
};

/* define a type AST node which will hold pointers to AST structs that will
   allow us to represent the parsed code */
struct ASTnodetype
{
   enum ASTtype type;
   enum OPERATORS operator;
   enum SYSTYPE istype;
   char * name;
   char * label;
   int value;
   struct SymbTab *Symbol; // pointer to the symbol table;
   struct ASTnodetype *next, *s1,*s2 ; /* used for holding IF and WHILE
                                       components -- not very descriptive */
};

typedef struct ASTnodetype ASTnode;

/* uses malloc to create an ASTnode and passes back the heap address
   of the newley created node */
ASTnode *ASTCreateNode(enum ASTtype mytype);

// helper function to indent the printout of the AST
void PT( int howmany );

void printType ( enum SYSTYPE type );

void ASTattachnext( ASTnode *p , ASTnode *q );

ASTnode *program;

/*  Print out the abstract syntax tree */
void ASTprint( int level , ASTnode *p );

#endif // of AST_H
