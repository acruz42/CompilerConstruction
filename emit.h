/*
Adrian Cruz
CS 370 - Lab 9
Function prototypes for emit.c
*/

#include<stdio.h>
#include<malloc.h>

#ifndef EMIT_H
#define EMIT_H
#include "ast.h"

char * GenLabel();
void blankline();
void emitAST( ASTnode *p );
void emit_global( ASTnode * p );
void emit_global_strings( ASTnode * p );
void emit( char * label , char * action , char * comment );
void emit_ident( ASTnode * p );
void emit_expr( ASTnode * p );
void emit_function_call( ASTnode * p );
void emit_if( ASTnode * p );
void emit_while( ASTnode * p );

#endif
