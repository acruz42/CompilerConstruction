/*

Adrian Cruz
CS 370 - Lab 9

This LEX protgram reads the stream coming in from the file and returns the appropriate
tokens. New regular expressions were added for special/reserved words, and not have them confused as ID tokens.

*/
%{

// variable to keep track of the line number
int lineNum = 1;

int mydebug = 0;

#include "ast.h"
#include "y.tab.h"


%}

/*
The following regular expressions are to allow lex to return the new tokens that are
required for the YACC grammar rules.
*/

%%

int                     { if ( mydebug ) fprintf( stderr, "INT token found\n" );
                            return( INT ); }

void                    { if ( mydebug ) fprintf( stderr, "VOID token found\n" );
                            return( VOID ); }

boolean                 { if ( mydebug ) fprintf( stderr, "BOOLEAN token found\n" );
                            return( BOOLEAN ); }

begin                   { if ( mydebug ) fprintf( stderr, "BEGIN token found\n" );
                            return( MYBEGIN ); }

end                     { if ( mydebug ) fprintf( stderr, "END token found\n" );
                            return( MYEND ); }

if                      { if ( mydebug ) fprintf( stderr, "IF token found\n" );
                            return( IF ); }

then                    { if ( mydebug ) fprintf( stderr, "THEN token found\n" );
                            return( THEN ); }

else                    { if ( mydebug ) fprintf( stderr, "ELSE token found\n" );
                            return( ELSE ); }

while                   { if ( mydebug ) fprintf( stderr, "WHILE token found\n" );
                            return( WHILE ); }

do                      { if ( mydebug ) fprintf( stderr, "DO token found\n" );
                            return( DO ); }

return                  { if ( mydebug ) fprintf( stderr, "RETURN token found\n" );
                            return( RETURN ); }

read                    { if ( mydebug ) fprintf( stderr, "READ token found\n" );
                            return( READ ); }

write                   { if ( mydebug ) fprintf( stderr, "WRITE token found\n" );
                            return( WRITE ); }

and                     { if ( mydebug ) fprintf( stderr, "AND token found\n" );
                            return( AND ); }

or                      { if ( mydebug ) fprintf( stderr, "OR token found\n" );
                            return( OR ); }

true                    { if ( mydebug ) fprintf( stderr, "TRUE token found\n" );
                            return( MYTRUE ); }

false                   { if ( mydebug ) fprintf( stderr, "FALSE token found\n" );
                            return( MYFALSE ); }

not                     { if ( mydebug ) fprintf( stderr, "NOT token found\n" );
                            return( NOT ); }

\".*\"                  { if ( mydebug ) fprintf( stderr, "STRING found\n" );
                          yylval.string = strdup( yytext ); return( STRING ); }

"<="                    { if ( mydebug ) fprintf( stderr, "<= token found\n" );
                            return( LE ); }

">="                    { if ( mydebug ) fprintf( stderr, ">= token found\n" );
                            return( GE ); }

"=="                    { if ( mydebug ) fprintf( stderr, "== token found\n" );
                            return( EQ ); }

"!="                    { if ( mydebug ) fprintf( stderr, "!= token found\n" );
                            return( NE ); }

[a-zA-Z][a-zA-Z0-9]*	{ if ( mydebug ) fprintf ( stderr, "ID token found\n" ); 
                            yylval.string = strdup( yytext ); return( ID ); }

[0-9][0-9]*	            { if ( mydebug ) fprintf ( stderr, "NUM token found\n" ); 
                            yylval.value = atoi( ( const char * ) yytext ); return( NUM ); }

[ \t]		            { if ( mydebug ) fprintf ( stderr, "Whitespace found\n" ); }

[=\-+*/%&|<>!();,""\[\]] 	{ if ( mydebug ) fprintf ( stderr, "Return the token %c\n" , *yytext );
                            return ( *yytext );
                            /* added the chars [ ] ; < > ! , ( ) so they could be used as tokens in YACC */
                        }

\n                      { if ( mydebug ) fprintf ( stderr, "Newline\n" ); lineNum++;
                             /* increase lineNum when a newline character is seen */ }

%%

int yywrap(void)
{ return 1;}
