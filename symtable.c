/*  Symbol Table --linked list
    Used for Compilers class

    Modified Spring 2015 to allow for name to pointed to by symtable, instead
    of copied, since the name is copied into the heap

    Modified to have levels.  A level 0 means global variable, other levels
    means in range of the function.  We start out our offsets at 0
    (from stack pointer) for level 1,,,when we enter a functional declaration.
    We increment offset each time we insert a new variable.  A variable is
    considered to be valid if it is found in the symbol table at our level or
    lesser level.  If at 0, then it is global.

    We return a pointer to the symbol table when a variable matches our criteria.

    We add a routine to remove variables at our level and above.
    
Adrian Cruz
Lab 9

function definitions to be used in YACC

*/

#include <string.h>
#include "symtable.h"
#include "ast.h"

int GTEMP = 0;

struct SymbTab *first, *last;

char * CreateTemp()
{
   char hold[100];
   char *s;
   sprintf(hold,"_T%d",GTEMP++);
   s=strdup(hold);
   return (s);
}

/* Simple Insert into the symbol table with the size,
   type level that the name is being inserted into */

struct SymbTab * Insert(char *name, enum SYSTYPE Type, int isafunc, int level, int mysize, int offset , ASTnode * fparms)
{
   struct SymbTab * n;
   n=Search(name,level, 0);
   if(n!=NULL)
   {
      printf("\n\tThe name %s exists at level %d already", name , level);
      printf(" in the symbol table\n\tDuplicate can't be inserted");
      return (NULL);
   }
   else
   {
      struct SymbTab *p;
      p=malloc(sizeof(struct SymbTab));
      p->name=name;
      p->offset=offset;  /* assign the offset */
      p->level=level;  /* assign the level */
      p->mysize=mysize;  /* assign the size */
      p->Type=Type;  /* assign the Type */
      p->IsAFunc=isafunc;  /* assign the Function  */
      p->fparms=fparms;  /* assign the Function  */
      p->next=NULL;

   /* Check on how many elements we have in the symbol table */
      if(first==NULL)
      {
         first=p;
      }
      else
      {
         p->next=first;
         first=p;
      }
      return (p);
    }

    printf("\n\tLabel inserted\n");
}

/* print out a single symbol table entry -- for debugging */
void PrintSym(struct SymbTab *s)
{
   printf("\t%s\t%d\t%d\t",s->name,s->offset, s->level);
   printType( s->Type );
   printf("\n");
}


/*  General display to see what is our symbol table */

void Display()
{
   int i;
   struct SymbTab *p;
   p=first;
   printf("\n\tLABEL\tOffset\tLEVEL\tType\n");
      while (p!=NULL)
      {
         PrintSym(p);
         p=p->next;
      }
}

/* Search for a symbol name at level or below.  We have to do multiple passes
   into the symbol table because we have to find the name closest to us

   If recur is non-zero, then we look through all of the levels,
   otherwise, only our level
   We return a pointer to a SymbolTab structure so that we can use other
   functions/methods to get the attributes */

struct SymbTab * Search(char *name, int level, int recur)
{
   int i,flag=0;
   struct SymbTab *p;

   /* for each level, try to find our symbol */
   while (level >= 0)
   {
      p=first;
      while (p!=NULL)
      {
         if((strcmp(p->name,name)==0) && (p->level == level))
            return p;
         p=p->next;
      }
      if (recur == 0)
         return (NULL);   /* we did not find it at our level */
      level--; /* check the next level up */
   }
   return  NULL;  /* did not find it, return 0 */
}

/* Remove all enteries that have the indicated level
   We need to take care about updating first pointer into the linked list
   when we are deleting edge elements */


int Delete(int level)
{
   struct SymbTab *p,*f=NULL;  /* we follow with pointer f */
   int SIZE=0;
   p=first;

   /* cruise through the list */

   while (p != NULL)
   {
      /* do we match? */
      if (p->level>= level )
      {
         /* if it is the first in the list we have to update first,
         we know this by f being NULL */
         SIZE+=p->mysize;
         if ( f==NULL)
            first=p->next;
         else /* not the first element */
         {
            f->next=p->next;
         }
         p=p->next;
      }
      else
      {
         /* update follow pointer, move the p pointer */
         f=p;
         p=p->next;
      }
   }
   return(SIZE);
}

/* Function to compare the arguments of a call to the paramaters of
   that function, return 0 if these do not match */
int compareForm ( ASTnode * fparms , ASTnode * aparms )
{
   // new pointers to move through paramList and argList
   ASTnode * p1, * p2;
   p1 = fparms;
   p2 = aparms;
   
   while ( p1 != NULL )
   {
      // wrong number of arguments
      if ( p2 == NULL )
         return 0;
      // type mismatch
      if ( p1->istype != p2->istype )
         return 0;      
      p1 = p1->next;
      p2 = p2->next;
   }

   // if we finished with the formal parameters
   // then there shouldn't be more arguments in the call
   if ( p2 != NULL )
      return 0;
   return 1;
}
