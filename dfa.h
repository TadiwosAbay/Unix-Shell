
#include "dynarray.h"
#ifndef DFA_INCLUDED
#define DFA_INCLUDED

/*--------------------------------------------------------------------*/

enum {MAX_LINE_SIZE = 1024};

enum {FALSE, TRUE};

enum TokenType {TOKEN_NUMBER, TOKEN_WORD, TOKEN_PIPE, TOKEN_INREDIR, 
TOKEN_OUTREDIR, TOKEN_BACKGROUND};

/*--------------------------------------------------------------------*/

/* A Token is either a number or a word, expressed as a string. */

struct Token
{
   enum TokenType eType;
   /* The type of the token. */

   char *pcValue;
   /* The string which is the token's value. */
};

/*--------------------------------------------------------------------*/
void freeToken(void *pvItem, void *pvExtra);
void printToken(void *pvItem, void *pvExtra);
struct Token *makeToken(enum TokenType eTokenType,
   char *pcValue);
int lexLine(const char *pcLine, DynArray_T oTokens);

#endif