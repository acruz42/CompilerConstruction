/*   Abstract syntax tree code

Adrian Cruz
CS 370 - Lab 9

Both emit_global and emit_global_strings functions are the first two
functions called. They print the global variables and strings from
the program.

emitAST is the recursive function that will take the beginnig of 
the program and print to a .asm file the generated code. Which will 
be dependant on the ASTnode that it receives.

*/

#include<stdio.h>
#include<malloc.h>
#include <string.h>
#include "emit.h"
#include "ast.h"
#include "symtable.h"

#define WSIZE 8
#define LOGWSIZE 3

extern int mydebug;

FILE * fp;
int LABEL = 0;
char hold[100];
char temp[100];

// helper function to create new labels
char * GenLabel()
{
   char *s;
   sprintf(hold,"_L%d",LABEL++);
   s=strdup(hold);
   return (s);
}

// function to help format
void blankline()
{
   emit( "" , "" , "" );
}

/* emit the instructions that come with IDENTs
 * precondition: p is an IDENT
 * postcondition: RAX will be the memory address where IDENT resides */
void emit_ident( ASTnode * p )
{
   // IDENT is an array
   if ( p->s1 != NULL )
   {
      // first solve the index of the array dependant on its type
      switch ( p->s1->type )
      {
         case NUMBER :
            sprintf( hold , "mov rbx, %d" , p->s1->value );
            emit( "" , hold , ";load immediate a number" );
            break;
         case IDENT :
            emit_ident( p->s1 );
            emit( "" , "mov rbx, [rax]" , ";move to RBX the value of IDENT" );
            break;
         case EXPR :
            emit_expr( p->s1 );
            sprintf( hold , "mov rbx, [rsp+%d]" , p->s1->Symbol->offset * WSIZE );
            emit( "" , hold , ";move to RBX the result of EXPR index" ); 
            break;
         case CALL :
            emit_function_call( p->s1 );
            emit( "" , "mov rbx, rax" , ";move to RBX the result of the CALL" );
            break;
      }
      // shift the value for array referencing
      sprintf( hold , "shl rbx, %d" , LOGWSIZE );
      emit( "" , hold , ";ARRAY reference needs WSIZE differencing" );
   }
   // get the addres of the IDENT if its global
   if ( p->Symbol->level == 0 )
   {
      sprintf( hold , "mov rax, %s" , p->name );
      emit( "" , hold , ";pass to rax the value of the global variable" );
   }
   // IDENT is not global
   else
   {
      sprintf( hold , "mov rax, %d" , p->Symbol->offset * WSIZE );
      emit( "" , hold , ";get offset of Identifier" );
      emit( "" , "add rax, rsp" , ";add SP to the offset for the address" );
   }
   // if there was an array, add the reference(index) to the memory location
   if ( p->s1 != NULL )
      emit( "" , "add rax, rbx" , ";add on RBX as this is an array reference" );
} // end emit_ident

/* emit the instructions for expressions 
 * precondition p is an EXPR node
 * postcondition the result will be stored in p->Symbol->offset */
void emit_expr( ASTnode * p )
{
   // LHS of an expression will be stored in RAX
   // solve the LHS dependant on its type
   switch ( p->s1->type )
   {
      case NUMBER :
         sprintf( hold , "mov rax, %d" , p->s1->value );
         emit( "" , hold , ";load immediate a number" );
         break;
      case IDENT :
         emit_ident( p->s1 );
         emit( "" , "mov rax, [rax]" , ";move to rax the value of IDENT on LHS" );
         break;
      case EXPR :
         emit_expr( p->s1 );
         sprintf( hold , "mov rax, [rsp+%d]" , p->s1->Symbol->offset * WSIZE );
         emit( "" , hold , ";move to rax the result of the LHS of EXPR" ); 
         break;
      case CALL :
         emit_function_call( p->s1 );
         // rax will already hold the desired value
         break;
      default:
         printf( "***ERROR***\n" );
         return;
   }
   // store in the temp Symbol offset the result of the LHS
   sprintf( hold , "mov [rsp+%d], rax" , p->Symbol->offset * WSIZE );
   emit( "" , hold , ";store the result of the LHS" );
   // the MYNOT operator is special since it is a unary operator
   // skip the RHS if it is a MYNOT
   if ( p->operator != MYNOT )
   {
      // RHS of an expression will be stored in RBX
      switch ( p->s2->type )
      {
         case NUMBER :
            sprintf( hold , "mov rbx, %d" , p->s2->value );
            emit( "" , hold , ";load immediate a number" );
            break;
         case IDENT :
            emit_ident( p->s2 );
            emit( "" , "mov rbx, [rax]" , ";move to rbx the value of IDENT on RHS" );
            break;
         case EXPR :
            emit_expr( p->s2 );
            sprintf( hold , "mov rbx, [rsp+%d]" , p->s2->Symbol->offset * WSIZE );
            emit( "" , hold , ";move to rbx the result of the RHS of EXPR" ); 
            break;
         case CALL :
            emit_function_call( p->s2 );
            // this function will store result in RAX
            // so we must move it to RBX
            emit( "" , "mov rbx, rax" , ";move to rax the value from the function call" );
            break;
         default:
            printf( "***ERROR***\n" );
            return;
      }
   }
   // since the operator was MYNOT set rbx to one for the comparison later
   else
   {
      emit( "" , "mov rbx, 1" , ";store 1 on RBX for the NOT compare" );
   }
   sprintf( hold , "mov rax, [rsp+%d]" , p->Symbol->offset * WSIZE );
   emit( "" , hold , ";fetch value of LHS of EXPR" );
   // determine the type of operation we must perform
   // first 4 are basic arithmetic operations
   switch ( p->operator )
   {
      case PLUS :
         emit( "" , "add rax, rbx" , ";perform addition" );
         break;
      case MINUS :
         emit( "" , "sub rax, rbx" , ";perform substraction" );
         break;
      case TIMES :
         emit( "" , "imul rbx" , ";perform multiplication" );
         break;
      case DIVIDE :
         emit( "" , "xor rdx, rdx" , ";necessary for divisions" );
         emit( "" , "idiv rbx" , ";perform division" );
         break;
      case EQUALTO :
         emit( "" , "cmp rax, rbx" , ";EQUALTO comparison" );
         emit( "" , "sete al" , ";set lower byte for equal" );
         emit( "" , "mov rbx, 1" , ";load 1 to RBX" );
         emit( "" , "and rax, rbx" , ";" );
         break;
      // let MYNOT fall through
      case MYNOT :
      // in these cases set the lower byte depending on the type of
      // comparison we want to perform, and do bitwise and with 1
      case NOTEQUAL :
         emit( "" , "cmp rax, rbx" , ";NOTEQUAL comparison" );
         emit( "" , "setne al" , ";set lower byte for not equal" );
         emit( "" , "mov rbx, 1" , ";load 1 to RBX" );
         emit( "" , "and rax, rbx" , ";" );
         break;
      case LESSTHAN :
         emit( "" , "cmp rax, rbx" , ";EQUALTO comparison" );
         emit( "" , "setl al" , ";set lower byte for less than" );
         emit( "" , "mov rbx, 1" , ";load 1 to RBX" );
         emit( "" , "and rax, rbx" , ";" );
         break;
      case LESSTHANEQ :
         emit( "" , "cmp rax, rbx" , ";EQUALTO comparison" );
         emit( "" , "setle al" , ";set lower byte for less than or equal" );
         emit( "" , "mov rbx, 1" , ";load 1 to RBX" );
         emit( "" , "and rax, rbx" , ";" );
         break;
      case GREATERTHAN :
         emit( "" , "cmp rax, rbx" , ";EQUALTO comparison" );
         emit( "" , "setg al" , ";set lower byte for greater than" );
         emit( "" , "mov rbx, 1" , ";load 1 to RBX" );
         emit( "" , "and rax, rbx" , ";" );
         break;
      case GREATERTHANEQ :
         emit( "" , "cmp rax, rbx" , ";EQUALTO comparison" );
         emit( "" , "setge al" , ";set lower byte for greater than or equal" );
         emit( "" , "mov rbx, 1" , ";load 1 to RBX" );
         emit( "" , "and rax, rbx" , ";" );
         break;
      case MYAND :
         emit( "" , "mov rcx, 1" , ";store 1 on RCX for upcoming bitwise AND" );
         emit( "" , "cmp rax, 0" , ";compare RAX to zero" );
         emit( "" , "setne al" , ";set lower byte" );
         emit( "" , "and rax, rcx" , ";bitwise AND, RAX now is either 1 or 0" );
         emit( "" , "cmp rbx, 0" , ";compare RBX to zero" );
         emit( "" , "setne bl" , ";set lower byte" );
         emit( "" , "and rbx, rcx" , ";bitwise AND, RBX now is either 1 or 0" );
         emit( "" , "and rax, rbx" , ";perform AND operation" );
         break;
      // basic or operator
      case MYOR :
         emit( "" , "or rax, rbx" , ";perform OR operation" );
         break;
   }
   sprintf( hold , "mov [rsp+%d], rax" , p->Symbol->offset * WSIZE );
   emit( "" , hold , ";move to pointer offset the result of EXPR" );
} // end emit_expr

/* emit for use with function calls
   precondition: p is a CALL node
   postcondition: RAX will hold the return value of the call */
void emit_function_call( ASTnode * p )
{
   // all argument offset start at two and increase by one
   // first two are RSP and RBP
   int argoffset = 2;
   ASTnode * args = p->s1;
   // for as long as there are args
   while ( args != NULL )
   {
      // solve each argument depending on type
      switch ( args->s1->type )
      {
         case NUMBER :
            sprintf( hold , "mov rax, %d" , args->s1->value );
            emit( "" , hold , ";CALL arg is a number" );
            break;
         case IDENT :
            emit_ident( args->s1 );
            emit( "" , "mov rax, [rax]" , ";CALL arg is an IDENT" );
            break;
         case EXPR :
            emit_expr( args->s1 );
            sprintf( hold , "mov rax, [rsp+%d]" , args->s1->Symbol->offset * WSIZE );
            emit( "" , hold , ";CALL arg is an EXPR" ); 
            break;
         case CALL :
            emit_function_call( args->s1 );
            // rax will already hold the desired value
            emit( "" , ";CALL's arg is another CALL" , "" );
            break;
         default:
            printf( "***ERROR***\n" );
            return;
      }
      // store in the args temp offset the value of said arg
      sprintf( hold , "mov [rsp+%d], rax" , args->Symbol->offset * WSIZE );
      emit( "" , hold , ";temporarily store argument in memory" );
      args = args->next;
   }
   // get our new offset for the future activation record
   emit( "" , "mov rbx, rsp" , ";copy stack pointer for call moves" );
   sprintf( hold , "sub rbx, %d" , ((Search(p->name,0,0)->mysize)+1) * WSIZE );
   emit( "" , hold , ";RBX is the new target to the function call" );

   // reset args and loop for all the args
   args = p->s1;
   while ( args != NULL )
   {
      // fetch the value previously stored and put it in the location
      // of the future activation record
      sprintf( hold , "mov rax, [rsp+%d]" , args->Symbol->offset * WSIZE );
      emit( "" , hold , ";fetch the value of the argument" );
      sprintf( hold , "mov [rbx+%d], rax" , argoffset * WSIZE );
      emit( "" , hold , "; copy RAX (arg) to RBX target" );
      argoffset++;
      args = args->next;
   }
   sprintf( hold , "call %s" , p->name );
   emit( "" , hold , ";call to function" );
}

/* function to emit the instructions of WHILE statements
   precondition: p is a WHILESTMT node
   postcondition: output NASM code for WHILESTMT */
void emit_while( ASTnode * p )
{
   char * L1;
   char * L2;
   L1 = GenLabel();
   L2 = GenLabel();

   sprintf( hold , "%s:" , L1 );
   emit( hold , "" , ";begin of WHILE" );
   // solve the WHILE expression, whatever it may be
   switch ( p->s1->type )
   {
      case NUMBER :
         sprintf( hold , "mov rax, %d" , p->s1->value );
         emit( "" , hold , ";load immediate a number" );
         break;
      case IDENT :
         emit_ident( p->s1 );
         emit( "" , "mov rax, [rax]" , ";move to rax the value of IDENT" );
         break;
      case EXPR :
         emit_expr( p->s1 );
         sprintf( hold , "mov rax, [rsp+%d]" , p->s1->Symbol->offset * WSIZE );
         emit( "" , hold , ";move to rax the result of the LHS of EXPR" ); 
         break;
      case CALL :
         emit_function_call( p->s1 ); // this function will store result in RAX
         break;
   }
   // compare to zero if equal jump out
   emit( "" , "cmp rax, 0" , ";compare if expression of while is true" );
   sprintf( hold , "je %s" , L2 );
   emit( "" , hold , ";exit the while loop" );
   emitAST( p->s2 );
   sprintf( hold , "jmp %s" , L1 );
   emit( "" , hold , ";jump back to WHILE loop" );
   sprintf( hold , "%s:" , L2 );
   emit( hold , "" , ";end of WHILE" );
}

/* function to emit the instructions of IF statements
   precondition: p is an IFSTMT node
   postcondition: output NASM code for IFSTMT */
void emit_if( ASTnode * p )
{
   char * L1;
   char * L2;
   L1 = GenLabel();
   L2 = GenLabel();

   switch ( p->s1->type )
   {
      // solve the IFSTMT expression, depending on its type
      case NUMBER :
         sprintf( hold , "mov rax, %d" , p->s1->value );
         emit( "" , hold , ";load immediate a number" );
         break;
      case IDENT :
         emit_ident( p->s1 );
         emit( "" , "mov rax, [rax]" , ";move to rax the value of IDENT" );
         break;
      case EXPR :
         emit_expr( p->s1 );
         sprintf( hold , "mov rax, [rsp+%d]" , p->s1->Symbol->offset * WSIZE );
         emit( "" , hold , ";move to rax the result of the LHS of EXPR" ); 
         break;
      case CALL :
         emit_function_call( p->s1 ); // this function will store result in RAX
         break;
   }
   // jump to the if false instructions
   emit( "" , "cmp rax, 0" , ";comparison of IF" );
   sprintf( hold , "je %s" , L1 );
   emit( "" , hold , ";jump to if false instructions" );
   // emit the block of true instructions
   emitAST( p->s2->s1 );
   // jump out of if
   sprintf( hold , "jmp %s" , L2 );
   emit( "" , hold , ";jump to end of if" );
   sprintf( hold , "%s:" , L1 );
   emit( hold , "" , ";false instructions" );
   // emit the block of false instructions
   emitAST( p->s2->s2 );
   sprintf( hold , "%s:" , L2 );
   emit( hold , "" , ";end of IF" );
} // end emit_if

// helper function to format printing into file
void emit( char * label , char * action , char * comment )
{
   fprintf( fp , "%-*s%-*s %s\n" , 7 , label , 21 , action , comment );
}

/* emit all global variables to asm file
 * precondition: p is the first ASTnode of the program
 * postcondition: output NASM code for all global variables */
void emit_global( ASTnode * p )
{
   if ( p == NULL )
      return;

   if ( p->type == VARDEC && p->Symbol->level == 0 )
   {
      sprintf( hold , "common %s %d" , p->name , ( WSIZE * p->Symbol->mysize ) );
      emit( "" , hold , ";define global variable" );
   }
   emit_global( p->s1 );
   emit_global( p->next );
   return;
} // end emit_global

/* emit strings to asm file
 * precondition: p is the first ASTnode of the program
 * postcondition: output NASM code for all strings in the program */
void emit_global_strings( ASTnode * p )
{
   if ( p == NULL )
      return;

   if ( p->type == WRITESTMT && p->name != NULL )
   {
      sprintf( temp , "%s:" , p->label );
      sprintf( hold , "db %s, 0" , p->name );
      emit( temp , hold , ";global string" );
   }
   emit_global_strings( p->s1 );
   emit_global_strings( p->s2 );
   emit_global_strings( p->next );
   return;
} // end emit_global_strings

// this function will emit the whole program after the globals have been completed
// precondition: p is the first ASTnode of the program
// postcondition: the whole program will be emitted
void emitAST( ASTnode *p )
{
   // do nothing in case node is null
   if (p == NULL )
      return;
   else
   {
      switch (p->type) {
         // global variables handled by emit_global
         // all other VARDEC are handled with offset
         case VARDEC :
            if ( mydebug )
               printf( "Inside VARDEC\n" );
            break;

         // emit NASM for FUNDECs
         case FUNDEC :
            blankline();
            if ( mydebug )
               printf( "Inside FUNDEC\n" );
            sprintf( hold , "%s:" , p->name );
            emit( hold , "" , ";start of a function" );
            // special case for MAIN function
            if ( strcmp( p->name , "main" ) == 0 )
               emit( "" , "mov rbp, rsp" , ";Special RSP to RBP for MAIN only" );
            emit( "" , "mov r8, rsp" , ";FUNC header RSP has to be at most RBP" );
            sprintf( hold , "add r8, -%d" , ( p->Symbol->mysize * WSIZE ) );
            emit( "" , hold , ";adjust Stack Pointer for Activation record" );
            emit( "" , "mov [r8], rbp" , ";FUNC header store old BP" );
            emit( "" , "mov [r8+8], rsp" , ";FUNC header store old SP" );
            emit( "" , "mov rsp, r8" , ";FUNC header new SP" );

            emitAST( p->s1 );
            emitAST( p->s2 );

            emit( "" , "mov rbp, [rsp]" , ";FUNC end restore old BP" );
            emit( "" , "mov rsp, [rsp+8]" , ";FUNC end restore old SP" );
            // special case for MAIN function
            if ( strcmp( p->name , "main" ) == 0 )
               emit( "" , "mov rsp, rbp" , ";stack and BP need to be same at end of MAIN" );
            emit( "" , "xor rax, rax" , ";rax is now equal to 0" );
            emit( "" , "ret" , "" );
            blankline();          
            break;

         case PARAM :
            if ( mydebug )
               printf( "Inside PARAM\n" );
            break;

         // print the instructions for blocks
         case BLOCK :
            if ( mydebug )
               printf( "Inside BLOCK\n" );
            emitAST( p->s1 );
            emitAST( p->s2 );
            break;


         case READSTMT :
            if ( mydebug )
               printf( "Inside READSTMT\n" );
            // get the address where we will store the value read
            emit_ident( p->s1 );
            sprintf( hold , "GET_DEC %d, [rax]" , WSIZE );
            emit( "" , hold , ";read a variable" );
            break;

         // NASM instructions for write statements
         case WRITESTMT :
            if ( mydebug )
               printf( "Inside WRITESTMT\n" );
            // if s1 is null there is no EXPR it's a string, print that string
            if ( p->s1 == NULL )
            {
               sprintf( hold , "PRINT_STRING %s" , p->label );
               emit( "" , hold , ";standar print a string" );
               emit( "" , "NEWLINE" , ";standard newline" );
            }
            else
            {
               // solve the EXPR depending on its type and store the value on RSI
               switch ( p->s1->type )
               {
                  case NUMBER :
                     sprintf( hold , "mov rsi, %d" , p->s1->value );
                     emit( "" , hold , ";basic load immediate for a number" );
                     break;
                  case IDENT :
                     emit_ident( p->s1 );
                     emit( "" , "mov rsi, [rax]" , ";move to RSI the value of the IDENT");
                     break;
                  case EXPR :
                     emit_expr( p->s1 );
                     sprintf( hold , "mov rsi, [rsp+%d]" , p->s1->Symbol->offset * WSIZE );
                     emit( "" , hold , ";move to RSI the result of EXPR" );
                     break;
                  case CALL :
                     emit_function_call( p->s1 );
                     emit( "" , "mov rsi, rax" , ";move to RSI the return from call" );
                     break;
                  default:
                     printf( "Not implemented\n" );
                     return;
               }
               // print out the value that the EXPR stored on RSI
               sprintf( hold , "PRINT_DEC %d, rsi" , WSIZE );
               emit( "" , hold , ";print the value in RSI" );
               emit( "" , "NEWLINE" , ";standard newline" );
            }
            break;

         case CALL :
            if ( mydebug )
               printf( "Inside CALL\n" );
            emit_function_call( p );
            break;

         // print return statements
         case RETURNSTMT :
            if ( mydebug )
               printf( "Inside RETURNSTMT\n" );
            // if the RETURNSTMT has an EXPR solve it and store the result on RAX
            // by the end of the function RAX will hold the desired value
            if ( p->s1 != NULL )
            {
               switch ( p->s1->type )
               {
                  case NUMBER :
                     sprintf( hold , "mov rax, %d" , p->s1->value );
                     emit( "" , hold , ";load immediate a number" );
                     break;
                  case IDENT :
                     emit_ident( p->s1 );
                     emit( "" , "mov rax, [rax]" , ";move to rax the value of IDENT on LHS" );
                     break;
                  case EXPR :
                     emit_expr( p->s1 );
                     sprintf( hold , "mov rax, [rsp+%d]" , p->s1->Symbol->offset * WSIZE );
                     emit( "" , hold , ";move to rax the result of the LHS of EXPR" ); 
                     break;
                  case CALL :
                     emit_function_call( p->s1 );
                     // rax already will hold the desired value
                     break;
                  default:
                     printf( "***ERROR***\n" );
                     return;
               }
            }
            // restore the RBP and RSP
            emit( "" , "mov rbp, [rsp]" , ";FUNC end restore old BP" );
            emit( "" , "mov rsp, [rsp+8]" , ";FUNC end restore old SP" );
            emit( "" , "ret" , ";standard function return" );
            break;

         case WHILESTMT :
            if ( mydebug )
               printf( "Inside WHILESTMT\n" );
            blankline();
            emit_while( p );
            blankline();
            break;

         case ASSIGNSTMT :
            if ( mydebug )
               printf( "Inside ASSIGNSTMT\n" );
            // first solve the RHS of the ASSIGNSTMT and store it in the temp offset
            switch ( p->s2->type )
            {
               case EXPR :
                  emit_expr( p->s2 );
                  sprintf( hold , "mov rax, [rsp+%d]" , p->s2->Symbol->offset * WSIZE );
                  emit( "" , hold , ";move to RAX the result of EXPR" );
                  sprintf( hold , "mov [rsp+%d], rax" , p->Symbol->offset * WSIZE );
                  emit( "" , hold , ";store value of RAX(RHS) in memory" );
                  break;
               case IDENT :
                  emit_ident( p->s2 );
                  emit( "" , "mov rax, [rax]" , ";get value of IDENT" );
                  sprintf( hold , "mov [rsp+%d], rax" , p->Symbol->offset * WSIZE );
                  emit( "" , hold , ";store in memory the value of IDENT" );
                  break;
               case NUMBER :
                  sprintf( hold , "mov rax, %d" , p->s2->value );
                  emit( "" , hold , ";basic load immediate for a number" );
                  sprintf( hold , "mov [rsp+%d], rax" , p->Symbol->offset * WSIZE );
                  emit( "" , hold , ";store value of RAX(RHS) in memory" );
                  break;
               case CALL :
                  emit_function_call( p->s2 );
                  sprintf( hold , "mov [rsp+%d], rax" , p->Symbol->offset * WSIZE );
                  emit( "" , hold , ";store value of RAX(RHS) in memory" );
                  break;
            }
            // get the address of the VAR on the LHS of the ASSIGNSTMT will be in RAX
            emit_ident( p->s1 );
            // fetch value of RHS and put on RCX
            sprintf( hold , "mov rcx, [rsp+%d]" , p->Symbol->offset * WSIZE );
            emit( "" , hold , ";fetch the value from the RHS of the ASSIGNSTMT" );
            // put the value fetched into the address of the IDENT(RAX)
            emit( "" , "mov [rax], rcx" , ";store on memory location of IDENT the value of RCX" );
            break;

         case IFSTMT :
            if ( mydebug )
               printf( "Inside IFSTMT\n" );
            blankline();
            emit_if( p );
            blankline();
            break;

         // special case of a semicolon with nothing else
         case EMPTYSTMT :
            emit( "" , "nop" , ";EMPTYSTMT no operation" );
            break;

         default: printf("unknown type in emitAST\n");
      }
      emitAST( p->next);
   }
} // end emitAST
