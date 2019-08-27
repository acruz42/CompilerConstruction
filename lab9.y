%{

/*

Adrian Cruz
CS 370 - Lab 9

This program sereves as the syntax analyzer for the input to make sure that
it is a sentence or series of sentences that are part of our language.

Depending of the tokens it receives and assuming these are in the right order
the program then performs semantic actions to start building an AST.

Using lab6 as a starting point now the program inserts into a symbol table
when declaring new variables provided they had not been declared at the same
level before. It also performs cecking when using variables and calling funcs
to makes sure they have been previously declared.
The program also performs type checking for expressions and assign statements

For Lab9 minimal changes were done to this file, I only added more temps and
Symbols to some ASTnodes.

*/

/* begin specs */

#include <stdio.h>
#include <ctype.h>
#include "symtable.h"
#include "ast.h"
#include "emit.h"

int yylex();

// global variable controlled to report in which line the error occurred
extern int lineNum;
extern int mydebug;
extern FILE * fp;

ASTnode * myprogram = NULL;

void yyerror (s)  /* Called by yyparse on error */
     char *s;
{
  // changed the next line to include the lineNum variable
  fprintf ( stderr, "%s on line %d\n" , s , lineNum );
}

// variables to help with symbol table insertion
int level = 0;
int offset = 0;
int goffset;
int maxOffset;

%}
/* defines the start symbol, what values come back from LEX
   and how the operators are associated  */

%start program

// declared an union type so lex can return both integers and strings
%union
{
    int value;
    char * string;
    enum OPERATORS operator;
    enum SYSTYPE istype;
    ASTnode * node;
}

// assigned the type from the union that corresponds to each token
%token <value> NUM
%token <string> ID STRING

// declared all the new tokens that are necessary for this grammar to execute
%token INT VOID BOOLEAN MYBEGIN MYEND IF THEN ELSE WHILE DO RETURN READ
%token WRITE AND OR MYTRUE MYFALSE NOT LE GE EQ NE

// delared the type for each of the rules of the grammar
%type <node> varList varDec decList dec program funDec params param paramList
%type <node> compoundStmt localDecs statementList statement expressionStmt
%type <node> selectionStmt iterationStmt assignmentStmt returnStmt readStmt
%type <node> writeStmt var expr args argList call factor term simpleExpr
%type <node> additiveExpr // prototypeDec
%type <operator> relop addop multop
%type <istype> typeSpecifier

%left '|'
%left '&'
%left '+' '-'
%left '*' '/' '%'
%left UMINUS

%%	/* end specs, begin rules */

/* Next follow all of the production rules from the grammar along with corresponding
   semantic actions */

program        :  decList
                  {
                     // first pointer of the program, will point to
                     // everything that comes after
                     myprogram = $1;
                  }
               ;

decList        :  dec { $$ = $1; /* pass the pointer to $$ */ }
               |  dec decList
                  {
                     // pass the pointer and set its next
                     $$ = $1;
                     $$->next = $2;
                  }
               ;

dec            :  varDec { $$ = $1; }
               |  funDec { $$ = $1; }
               ;
/*
prototypeDec   :  typeSpecifier ID '(' params ')' compoundStmt ';'
                  {
                     
                  }
               ;
*/
varDec         :  typeSpecifier varList ';'
                  {
                     $$ = $2;
                     ASTnode * p;
                     p = $2;
                     // update the type of the node and symbtable struct
                     while ( p != NULL )
                     {
                        p->istype = $1;
                        p->Symbol->Type = $1;
                        p = p->s1;
                     }
                  }
               ;

varList        :  ID
                  {
                     // check that the variable hasn't been declared yet
                     struct SymbTab *p = Search( $1 , level , 0 );
                     if ( p == NULL || ( p != NULL && p->level != level ) )
                     {
                        // create a new node with its proper parts
                        $$ = ASTCreateNode(VARDEC);
                        $$->Symbol = Insert( $1 , 0 , 0 , level , 1 , offset , NULL );
                        $$->name = $1;
                        offset++;
                     }
                     else
                     {
                        fprintf( stderr , "***ERROR: Variable %s at level %d already declared***\n" , $1 , level );
                        fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                        return;
                     }
                  }

               |  ID '[' NUM ']'
                  {
                     // check that variable hasn't been declared yet
                     struct SymbTab *p = Search( $1 , level , 0 );
                     if ( p == NULL || ( p != NULL && p->level != level ) )
                     {
                        // create a new node with its proper parts
                        $$ = ASTCreateNode(VARDEC);
                        $$->Symbol = Insert( $1 , 0 , 2 , level , $3 , offset , NULL);
                        $$->name = $1;
                        $$->value = $3;
                        offset+= $3;
                     }
                     else
                     {
                        fprintf( stderr , "***ERROR: Variable %s at level %d already declared***\n" , $1 , level );
                        fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                        return;
                     }
                  }

               |  ID ',' varList
                  {
                     // check that variable hasn't been declared yet
                     struct SymbTab *p = Search( $1 , level , 0 );
                     if ( p == NULL || ( p != NULL && p->level != level ) )
                     {  
                        // create a new node with its proper parts
                        $$ = ASTCreateNode(VARDEC);
                        $$->Symbol = Insert( $1 , 0 , 0 , level , 1 , offset , NULL );
                        $$->name = $1;
                        $$->s1 = $3;
                        offset++;
                     }
                     else
                     {
                        fprintf( stderr , "***ERROR: Variable %s at level %d already declared***\n" , $1 , level );
                        fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                        return;
                     }
                  }

               |  ID '[' NUM ']' ',' varList
                  {
                     // check that variable hasn't been declared yet
                     struct SymbTab *p = Search( $1 , level , 0 );
                     if ( p == NULL || ( p != NULL && p->level != level ) )
                     {
                        // create a new node with its proper parts
                        $$ = ASTCreateNode(VARDEC);
                        $$->Symbol = Insert( $1 , 0 , 2 , level , $3 , offset , NULL);
                        $$->name = $1;
                        $$->value = $3;
                        $$->s1 = $6;
                        offset+= $3;
                     }
                     else
                     {
                        fprintf( stderr , "***ERROR: Variable %s at level %d already declared***\n" , $1 , level );
                        fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                        return;
                     }
                  }
               ;

               /* set the type for the nodes and symbtable struct */
typeSpecifier  :  INT     { $$ = INTTYPE; }
               |  VOID    { $$ = VOIDTYPE; }
               |  BOOLEAN { $$ = BOOLTYPE; }
               ;

funDec         :  typeSpecifier ID '(' 
                  {
                     // adjust the offsets at beginning of a FUNDEC
                     goffset = offset;
                     offset = 2;
                     maxOffset = offset;
                     // check that variable hasn't been declared yet
                     struct SymbTab *p = Search( $2 , level , 1 );
                     if ( p == NULL )
                     {
                        Insert( $2 , $1 , 1 , level , 0 , 0 , NULL );
                     }
                     else
                     {
                        fprintf( stderr , "***ERROR: Function %s is already declared***\n" , $2 );
                        fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                        return;
                     }
                  } 
                  params ')'
                  {
                     (Search( $2 , level , 1 ))->fparms = $5;
                  }
                  compoundStmt
                  {
                     // create new FUNDEC node and set its
                     // aspects accordingly
                     $$ = ASTCreateNode(FUNDEC);
                     $$->name = $2;
                     $$->istype = $1;
                     $$->Symbol = Search( $2 , level , 1 );
                     // the size of the BLOCK is the largest offset so far
                     $$->Symbol->mysize = maxOffset;
                     $$->value = maxOffset;
                     $$->label = $2;
                     $$->s1 = $5;
                     $$->s2 = $8;
                     // reset the offset to the global offset that keeps track
                     offset = goffset;
                  }
               ;

params         :  VOID      { $$ = NULL; }
               |  paramList { $$ = $1; }
               ;

paramList      :  param { $$ = $1; }
               |  param ',' paramList
                  {
                     $$ = $1;
                     $$->next = $3;
                  }
               ;

                  // create param nodes for printing in function declarations
param          :  typeSpecifier ID
                  {
                     // ensure the parameter name hasn't been used at that level before
                     struct SymbTab *p = Search( $2 , level + 1 , 0 );
                     if ( p == NULL )
                     {
                        Insert( $2 , $1 , 0 , level + 1 , 1 , offset , NULL );
                        $$ = ASTCreateNode(PARAM);
                        $$->istype = $1;
                        $$->name = $2;
                        offset++;
                     }
                     else
                     {
                        fprintf( stderr , "***ERROR: parameter %s already declared***\n" , $2 );
                        fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                        return;
                     }
                  }

               |  typeSpecifier ID '[' ']'
                  {
                     // ensure the parameter name hasn't been used at that level before
                     struct SymbTab *p = Search( $2 , level + 1 , 0 );
                     if ( p == NULL )
                     {
                        Insert( $2 , $1 , 2 , level + 1 , 1 , offset , NULL );
                        $$ = ASTCreateNode(PARAM);
                        $$->istype = $1;
                        $$->name = $2;
                        $$->value = 1;
                        offset++;
                     }
                     else
                     {
                        fprintf( stderr , "***ERROR: parameter %s already declared***\n" , $2 );
                        fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                        return;
                     }
                  }
               ;

               // create block nodes, blocks are everything inside a function
               // level is increased at beginning of the compound statement
compoundStmt   :  MYBEGIN { level++; } localDecs statementList MYEND
                  {
                     $$ = ASTCreateNode(BLOCK);
                     $$->s1 = $3;
                     $$->s2 = $4;
                     // keep track of the largest offset that will be needed
                     if (offset > maxOffset )
                        maxOffset = offset;
                     if ( mydebug )
                        Display();
                     // delete from symbol table variables at this level and higher
                     // also adjust offset accordingly
                     offset -= Delete(level);
                     // reduce level
                     level--;
                  }
               ;

               // declare local variables
localDecs      :  varDec localDecs
                  {
                     $$ = $1;
                     $$->s1 = $2;
                  }
               |  /* empty */ { $$ = NULL; }
               ;

               // recursively 'link' all statements together, via pointers
statementList  :  statement statementList
                  {
                     $$ = $1;
                     $$->next = $2;
                  }
               |  /* empty */ { $$ = NULL; }
               ;

               // all types of statements
statement      :  expressionStmt { $$ = $1; }
               |  compoundStmt   { $$ = $1; }
               |  selectionStmt  { $$ = $1; }
               |  iterationStmt  { $$ = $1; }
               |  assignmentStmt { $$ = $1; }
               |  returnStmt     { $$ = $1; }
               |  readStmt       { $$ = $1; }
               |  writeStmt      { $$ = $1; }
               ;

expressionStmt :  expr ';' { $$ = $1; }
               |  ';'
                  {
                     // special node for this case when
                     // there is nothing but a semicolon
                     $$ = ASTCreateNode(EMPTYSTMT);
                  }
               ;

selectionStmt  :  IF expr THEN statement
                  {
                     // if statements requre two nodes
                     // s1 of first node is everything that happens
                     // after the condition expression
                     $$ = ASTCreateNode(IFSTMT);
                     $$->s1 = $2;
                     // create second node accordingly
                     $$->s2 = ASTCreateNode(THENSTMT);
                     $$->s2->s1 = $4;
                  }

               |  IF expr THEN statement ELSE statement
                  {
                     // repeat from previous semantic action
                     $$ = ASTCreateNode(IFSTMT);
                     $$->s1 = $2;
                     $$->s2 = ASTCreateNode(THENSTMT);
                     $$->s2->s1 = $4;
                     // in this case if there is an else
                     // the s2 is not NULL
                     $$->s2->s2 = $6;
                  }
               ;

               // create the while nodes
iterationStmt  :  WHILE expr DO statement
                  {
                     $$ = ASTCreateNode(WHILESTMT);
                     $$->s1 = $2;
                     $$->s2 = $4;
                  }
               ;

returnStmt     :  RETURN expr ';'
                  {
                     $$ = ASTCreateNode(RETURNSTMT);
                     $$->s1 = $2;
                  }

               |  RETURN ';'
                  {
                     $$ = ASTCreateNode(RETURNSTMT);
                  }
               ;

readStmt       :  READ var ';'
                  {
                     $$ = ASTCreateNode(READSTMT);
                     $$->s1 = $2;
                  }
               ;

writeStmt      :  WRITE expr ';'
                  {
                     $$ = ASTCreateNode(WRITESTMT);
                     $$->s1 = $2;
                  }
               |  WRITE STRING ';'
                  {
                     $$ = ASTCreateNode(WRITESTMT);
                     $$->name = $2;
                     $$->label = GenLabel();
                  }
               ;

assignmentStmt :  var '=' simpleExpr ';'
                  {
                     // we cannot perform operations/expressions to void
                     if ( $1->istype == VOIDTYPE )
                     {
                        fprintf( stderr , "***ERROR: invalid VOID type in assignment***\n" );
                        fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                        return;
                     }
                     // make sure that the types match
                     if ( $1->istype != $3->istype )
                     {
                        fprintf( stderr , "***ERROR: assignment type mismatch***\n" );
                        fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                        return;
                     }
                     $$ = ASTCreateNode(ASSIGNSTMT);
                     $$->s1 = $1;
                     $$->s2 = $3;
                     $$->name = CreateTemp();
                     $$->istype = $1->istype;
                     $$->Symbol = Insert( $$->name , $$->istype , 0 , level , 1 , offset , NULL );
                     offset++;
                  }
               ;

expr           :  simpleExpr { $$ = $1; } ;

var            :  ID '[' expr ']'
                  {
                     // variable must have been declared previously
                     struct SymbTab *p = Search( $1 , level , 1 );
                     if ( p!= NULL && p->level <= level )
                     {
                        // make sure ID was inserted as an array
                        if ( p->IsAFunc == 2 )
                        {
                           $$ = ASTCreateNode(IDENT);
                           $$->s1 = $3;
                           $$->name = $1;
                           $$->istype = p->Type;
                           $$->Symbol = p;
                        }
                        else
                        {
                           fprintf( stderr , "***ERROR: variable %s is not an array***\n" , $1 );
                           fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                           return;
                        }
                     }
                     else
                     {
                        fprintf( stderr , "***ERROR: variable %s is not declared***\n" , $1 );
                        fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                        return;
                     }
                  }
               |  ID
                  {
                     // variable must have been declared previously
                     struct SymbTab *p = Search( $1 , level , 1 );
                     if ( p!= NULL && p->level <= level )
                     {
                        // if ID is not inserted as a scaler BARF
                        if ( p->IsAFunc == 0 )
                        {
                           $$ = ASTCreateNode(IDENT);
                           $$->name = $1;
                           $$->istype = p->Type;
                           $$->Symbol = p;
                        }
                        else
                        {
                           fprintf( stderr , "***ERROR: variable %s is an array***\n" , $1 );
                           fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                           return;
                        }
                     }
                     else
                     {
                        fprintf( stderr , "***ERROR: variable %s is not declared***\n" , $1 );
                        fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                        return;
                     }
                  }
               ;

               // left recursive so expressions are left to right
simpleExpr     :  additiveExpr { $$ = $1; }
               |  simpleExpr relop additiveExpr
                  {
                     // type checking
                     if ( $1->istype != $3->istype )
                     {
                        fprintf( stderr , "***ERROR: relational operation type mismatch***\n" );
                        fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                        return;
                     }
                     // we cannot perform operations/expressions to void
                     if ( $1->istype == VOIDTYPE || $3->istype == VOIDTYPE )
                     {
                        fprintf( stderr , "***ERROR: invalid VOID type in expression***\n" );
                        fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                        return;
                     }
                     $$ = ASTCreateNode(EXPR);
                     $$->s1 = $1;
                     $$->s2 = $3;
                     // this new node has the same type as the nodes of its expression
                     $$->istype = $1->istype;
                     $$->operator = $2;
                     // create a temp that will hold the result of the expression
                     $$->name = CreateTemp();
                     $$->Symbol = Insert( $$->name , $$->istype , 0 , level , 1 , offset , NULL );
                     offset++;
                  }
               ;

               // set the relational operator for the expression
relop          :  LE    { $$ = LESSTHANEQ; }
               |  '<'   { $$ = LESSTHAN; }
               |  '>'   { $$ = GREATERTHAN; }
               |  GE    { $$ = GREATERTHANEQ; }
               |  NE    { $$ = NOTEQUAL; }
               |  EQ    { $$ = EQUALTO; }
               ;

               // left recursive so expressions are left to right
additiveExpr   :  term { $$ = $1; }
               |  additiveExpr addop term
                  {
                     // type checking
                     if ( $1->istype != $3->istype )
                     {
                        fprintf( stderr , "***ERROR: additive type mismatch***\n" );
                        fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                        return;
                     }
                     // we cannot perform operations/expressions to void
                     if ( $1->istype == VOIDTYPE || $3->istype == VOIDTYPE )
                     {
                        fprintf( stderr , "***ERROR: invalid VOID type in expression***\n" );
                        fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                        return;
                     }
                     $$ = ASTCreateNode(EXPR);
                     $$->s1 = $1;
                     $$->s2 = $3;
                     // this new node has the same type as the nodes of its expression
                     $$->istype = $1->istype;
                     $$->operator = $2;
                     // create a temp that will hold the result of the expression
                     $$->name = CreateTemp();
                     $$->Symbol = Insert( $$->name , $$->istype , 0 , level , 1 , offset , NULL );
                     offset++;
                  }
               ;

               // set the additive operator/s for the expression/s
addop          :  '+' { $$ = PLUS; }
               |  '-' { $$ = MINUS; }
               ;

               // left recursive so expressions are left to right
term           :  factor { $$ = $1; }
               |  term multop factor
                  {
                     // type checking
                      if ( $1->istype != $3->istype )
                     {
                        fprintf( stderr , "***ERROR: multop type mismatch***\n" );
                        fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                        return;
                     }
                     // we cannot perform operations/expressions to void
                     if ( $1->istype == VOIDTYPE || $3->istype == VOIDTYPE )
                     {
                        fprintf( stderr , "***ERROR: invalid VOID type in expression***\n" );
                        fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                        return;
                     }
                     $$ = ASTCreateNode(EXPR);
                     $$->s1 = $1;
                     $$->s2 = $3;
                     // this new node has the same type as the nodes of its expression
                     $$->istype = $1->istype;
                     $$->operator = $2;
                     // create a temp that will hold the result of the expression
                     $$->name = CreateTemp();
                     $$->Symbol = Insert( $$->name , $$->istype , 0 , level , 1 , offset , NULL );
                     offset++;
                  }
               ;

               // set the operator/s for the expression/s
multop         :  '*'   { $$ = TIMES; }
               |  '/'   { $$ = DIVIDE; }
               |  AND   { $$ = MYAND; }
               |  OR    { $$ = MYOR; }
               ;

factor         :  '(' expr ')' { $$ = $2; }
               |  NUM
                  {
                     $$ = ASTCreateNode(NUMBER);
                     $$->istype = INTTYPE;
                     $$->value = $1;
                  }
               |  var   { $$ = $1; }
               |  call  { $$ = $1; }
               |  MYTRUE
                  {
                     $$ = ASTCreateNode(NUMBER);
                     $$->value = 1;
                     $$->istype = BOOLTYPE;
                     // use this operator for special case
                     // of true in ASTprint
                     $$->operator = TRU;
                  }

               |  MYFALSE
                  {
                     $$ = ASTCreateNode(NUMBER);
                     $$->value = 0;
                     $$->istype = BOOLTYPE;
                     // use this operator for special case
                     // of false in ASTprint
                     $$->operator = FAL;
                  }

               |  NOT factor
                  {
                     $$ = ASTCreateNode(EXPR);
                     $$->istype = BOOLTYPE;
                     $$->operator = MYNOT;
                     $$->s1 = $2;
                     $$->name = CreateTemp();
                     $$->Symbol = Insert( $$->name , $$->istype , 0 , level , 1 , offset , NULL );
                     offset++;
                  }
               ;

call           :  ID '(' args ')'
                  {
                     struct SymbTab *p = Search( $1 , level , 1 );
                     // ensure ID is in the symbol table
                     if ( p != NULL )
                     {
                        // make sure it is declared as a function
                        if ( p->IsAFunc == 1 )
                        {
                           // compare the parameters of p to the arguments in the call
                           if ( compareForm( p->fparms , $3 ) )
                           {
                              // create a new call node
                              $$ = ASTCreateNode(CALL);
                              $$->name = $1;
                              $$->Symbol = p;
                              $$->s1 = $3;
                              // give the call node the type of the function p
                              $$->istype = p->Type;
                           }
                           else
                           {
                              fprintf( stderr , "***ERROR: arguments mismatch***\n" );
                              fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                              return;
                           }
                        }
                        else
                        {
                           fprintf( stderr , "***ERROR: %s is not a function***\n" , $1 );
                           fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                           return;
                        }
                     }
                     else
                     {
                        fprintf( stderr , "***ERROR: function %s is not declared***\n" , $1 );
                        fprintf( stderr , "***ERROR: on line %d ***\n" , lineNum );
                        return;
                     }
                  }
               ;

args           :  argList     { $$ = $1; }
               |  /* empty */ { $$ = NULL; }
               ;

argList        :  expr
                  {
                     $$ = ASTCreateNode(ARGLIST);
                     $$->s1 = $1;
                     // give the node the SYSTYPE of the expression for type checking
                     $$->istype = $$->s1->istype;
                     $$->next = NULL;
                     $$->name = CreateTemp();
                     $$->Symbol = Insert( $$->name , $$->istype , 0 , level , 1 , offset , NULL );
                     offset++;
                  }
               |  expr ',' argList
                  {
                     $$ = ASTCreateNode(ARGLIST);
                     $$->s1 = $1;
                     // give the node the SYSTYPE of the expression for type checking
                     $$->istype = $$->s1->istype;
                     $$->next = $3;
                     $$->name = CreateTemp();
                     $$->Symbol = Insert( $$->name , $$->istype , 0 , level , 1 , offset , NULL );
                     offset++;
                  }
               ;

%%	/* end of rules, start of program */

int main( int argc , char * argv[] )
{
   for( int i = 0 ; i < argc ; i++ )
   {
      if ( strcmp( argv[i] , "-o" ) == 0 )
      {
         if ( strcmp( argv[ i + 1 ] , "-d" ) == 0 ||
              strcmp( argv[ i + 1 ] , "-o" ) == 0 ||
              argv[ i + 1 ] == NULL )
         {
            fprintf( stderr , "***ERROR choose a proper name for the file***\n" );
            return;
         }
         char * s1 = argv[ i + 1 ];
         char * s2 = ".asm";
         char * fileName = malloc( 1 + strlen(s1) + strlen(s2) );
         strcpy( fileName , s1 );
         strcat( fileName , s2 );
         fp = fopen( fileName , "w" );
         if ( fp == NULL )
         {
            fprintf( stderr , "***ERROR opening file***\n" );
            return;
         }
      }
      else if ( strcmp( argv[i] , "-d" ) == 0 )
         mydebug = 1;
   }
   // create a file with a default name in case the user didnt choose one
   if ( fp == NULL )
      fp = fopen( "Lab9.asm" , "w" );
   if ( fp == NULL )
   {
      fprintf( stderr , "***ERROR opening file***\n" );
      return;
   }

   yyparse();

   if ( mydebug )
   {
      printf( "\nMain Symbol Table\n" );
      Display();
      printf( "\n\nPrinting the AST\n\n" );
      ASTprint( 0 , myprogram );
   }

   // emit the global variables and strings onto the .asm file
   fprintf( fp , "%%include \"io64.inc\"\n\n" );
   emit_global( myprogram );
   fprintf( fp , "section .data\n\n" );
   emit_global_strings( myprogram );
   fprintf( fp , "section .text\n" );
   fprintf( fp , "%*sglobal main\n" , 7 , "" );

   // from here on out emit should be done so recursively
   emitAST( myprogram );
}
