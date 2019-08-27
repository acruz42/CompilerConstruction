/*   Abstract syntax tree code

Adrian Cruz
CS 370 - Lab 9

No changes were done to this file compared to lab6

This code is used to define an AST node,
routine for printing out the AST defining an enumerated type
to figure out what needs to be done with this.
The ENUM is basically going to be every non-terminal
and terminal in the language.

*/

#include<stdio.h>
#include<malloc.h>
#include "ast.h"


/* uses malloc to create an ASTnode and passes back the heap address
   of the newley created node */
ASTnode *ASTCreateNode(enum ASTtype mytype)
{
   if ( mydebug1 )
      fprintf( stderr,"Creating AST Node \n" );

   // all the components of each node
   ASTnode *p;
   p = ( ASTnode *) malloc( sizeof(ASTnode) );
   p->type = mytype;
   p->next = NULL;
   p->s1 = NULL;
   p->s2 = NULL;
   p->value = 0;
   return ( p );
}

/*  Helper function to print tabbing */
void PT(int howmany)
{
   for( int i = 1 ; i <= howmany ; i++ )
      printf( "  " );
}

/* Function to print out the type of the node */
void printType ( enum SYSTYPE type )
{
   switch (type) {
      case VOIDTYPE:
         printf( "void" );
         break;
      case INTTYPE:
         printf( "int" );
         break;
      case BOOLTYPE:
         printf( "boolean" );
         break;
      default:
         printf("unknown type in printType\n");
   }
}

/* Print out the abstract syntax tree
   depending on the type of the node perform the appropriate actions.
   Every type of node makes recursive calls to its s1, s2, next
   or neither accordingly */
void ASTprint( int level , ASTnode *p )
{
   // do nothing in case node is null
   if (p == NULL )
      return;
   else
   {
      PT(level); /* indent */
      switch (p->type) {

         // handle the printing of variable declarations
         case VARDEC :
            printf( "Variable " );
            // the type of variable
            printType( p->operator );
            printf( " %s" , p->name );
            if (p->value > 0)
               printf("[ %d ]" , p->value );
            printf( "\n" );
            ASTprint( level , p->s1 );
            break;

         // print function declaration nodes
         case FUNDEC :
            // the return type of the function
            printType( p->operator );
            printf( " function %s\n" , p->name );
            // print the paramater/s instructions handled elsewhere
            if ( p->s1 == NULL ) {
               PT( level + 1 );
               printf( "(VOID)\n" );
            }
            else {
               PT( level + 1 );
               printf( "(\n" );
               ASTprint( level + 1 , p->s1 );
               PT( level + 1 );
               printf( ")\n" );
            }
            // print the block inside the function
            ASTprint( level + 1 , p->s2 );
            PT( level );
            // helper to let me know when a FUNDEC has ended printing
            printf( "end FUNCTION %s\n\n" , p->name );
            break;

         // print parameters of a FUNDEC
         case PARAM :
            // type of the parameter
            printf( "Parameter " );
            printType( p->operator );
            printf( "%s" , p->name );
            // in case the paramater is an array
            if ( p->value != 0 )
               printf( " []" );
            printf( "\n" );
            break;

         // print the instructions inside a block
         case BLOCK :
            printf( "Block Statement\n" );
            ASTprint( level + 1 , p->s1 );
            ASTprint( level + 1 , p->s2 );
            PT( level );
            // helper to let me know when a block has ended printing
            printf( "end BLOCK\n" );
            break;


         case READSTMT :
            printf( "Read Statement\n" );
            ASTprint( level + 1 , p->s1 );
            break;

         // print Identifiers
         case IDENT :
            printf( "Identifier %s\n" , p->name );
            // handle array references
            if ( p->s1 != NULL ) {
               PT( level );
               printf( "Array reference [\n" );
               ASTprint( level + 1 , p->s1 );
               PT( level );
               printf( "] end ARRAY\n" );
            }
            break;

         // print write statements
         case WRITESTMT :
            printf( "Write Statement\n" );
            if ( p->s1 != NULL )
               ASTprint( level + 1 , p->s1 );
            else {
               PT( level + 1 );
               printf( "%s\n" , p->name);
            }
            break;

         // print calls to functions
         case CALL :
            printf( "Call function %s\n" , p->name );
            PT( level + 1 );
            printf( "(\n");
            ASTprint( level + 1 , p->s1 );
            PT( level + 1 );
            printf( ") end CALL\n");
            break;

         // print number nodes with their values
         case NUMBER :
            // special case for true
            if ( p->operator == TRU )
               printf( "True value\n" );
            // special case for false
            else if ( p->operator == FAL )
               printf( "False value\n" );
            else
               printf( "Number with value %d\n" , p->value );
            break;

         // print return statements
         case RETURNSTMT :
            printf( "Return Statement\n" );
            // special case nothing after return
            if ( p->s1 == NULL ) {
               PT( level + 1 );
               printf( "No Expression\n" );
            }
            // anything else
            else
               ASTprint( level + 1 , p->s1 );
            break;

         // print while loops
         case WHILESTMT :
            printf("While Statement\n");
            ASTprint( level + 1 , p->s1 );
            PT( level );
            printf( "Do\n" );
            ASTprint( level + 1 , p->s2 );
            PT( level );
            // helper to let me know when a while statement ends printing
            printf( "end WHILE\n" );
            break;

         // print Assignment statements
         case ASSIGNSTMT :
            printf( "Assignment Statement\n");
            ASTprint( level + 1 , p->s1 );
            PT( level + 1 );
            printf( "Equals\n");
            ASTprint( level + 2 , p->s2 );
            break;

         // print expressions
         case EXPR :
            printf( "Expression" );
            // print the type of operator for the expression
            switch( p->operator ) {
               case PLUS :
                  printf( " +" );
                  break;
               case MINUS :
                  printf( " -" );
                  break;
               case TIMES :
                  printf( " *" );
                  break;
               case DIVIDE :
                  printf( " /" );
                  break;
               case MYNOT :
                  printf( " not" );
                  break;
               case EQUALTO :
                  printf( " ==" );
                  break;
               case NOTEQUAL :
                  printf( " !=" );
                  break;
               case LESSTHAN :
                  printf( " <" );
                  break;
               case LESSTHANEQ :
                  printf( " <=" );
                  break;
               case GREATERTHAN :
                  printf( " >" );
                  break;
               case GREATERTHANEQ :
                  printf( " >=" );
                  break;
               case MYAND :
                  printf( " and" );
                  break;
               case MYOR :
                  printf( " or" );
                  break;
            }
            printf("\n");
            ASTprint( level + 1 , p->s1 );
            //if ( p->operator != MYNOT )
            ASTprint( level + 1 , p->s2 );
            break;

         // handle If nodes
         case IFSTMT :
            printf( "If Statement\n" );
            ASTprint( level + 1 , p->s1 );
            ASTprint( level , p->s2 );
            PT( level );
            // helper to know when print has ended
            printf( "end IF\n" );
            break;

         // print the second part of if statements
         case THENSTMT :
            printf( "Then:\n" );
            ASTprint( level + 1 , p->s1 );
            // special case there is an else statement
            if ( p->s2 != NULL ) {
               PT( level );
               printf( "Else:\n" );
               ASTprint( level + 1 , p->s2 );
            }
            break;

         // special case of a semicolon with nothing else
         case EMPTYSTMT :
            printf( "empty statement\n" );
            break;

         default: printf("unknown type in ASTprint\n");
      }
      ASTprint( level , p->next);
   }
} // end ASTprint
