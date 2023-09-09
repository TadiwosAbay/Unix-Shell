/*--------------------------------------------------------------------*/
/* dfa.c                                                              */
/* Original Author: Bob Dondero                                       */
/* Illustrate lexical analysis using a deterministic finite state     */
/* automaton (DFA)                                                    */
/*--------------------------------------------------------------------*/

#include "dynarray.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "dfa.h"

void freeToken(void *pvItem, void *pvExtra)

/* Free token pvItem.  pvExtra is unused. */

{
   struct Token *psToken = (struct Token*)pvItem;
   free(psToken->pcValue);
   free(psToken);
}

/*--------------------------------------------------------------------*/

void printToken(void *pvItem, void *pvExtra)

/* Print token pvItem to stdout.  pvExtra is
   unused. */

{
   struct Token *psToken = (struct Token*)pvItem;
   if (psToken->eType == TOKEN_NUMBER)
      printf("Number: %s\n", psToken->pcValue);
   else if (psToken->eType == TOKEN_WORD)
      printf("Word: %s\n", psToken->pcValue);
   else if (psToken->eType == TOKEN_PIPE)
      printf("Pipe: %s\n", psToken->pcValue);
   else if (psToken->eType == TOKEN_INREDIR)
      printf("Redirection_In: %s\n", psToken->pcValue);
   else if (psToken->eType == TOKEN_OUTREDIR)
      printf("Redirection_Out: %s\n", psToken->pcValue);
   else if (psToken->eType == TOKEN_BACKGROUND)
      printf("Background: %s\n", psToken->pcValue);
   
}


/*--------------------------------------------------------------------*/

struct Token *makeToken(enum TokenType eTokenType,
   char *pcValue)

/* Create and return a Token whose type is eTokenType and whose
   value consists of string pcValue.  Return NULL if insufficient
   memory is available.  The caller owns the Token. */

{
   struct Token *psToken;

   psToken = (struct Token*)malloc(sizeof(struct Token));
   if (psToken == NULL) {
      free(psToken);
      fprintf(stderr, "Cannot allocate memory\n");
      return NULL;
   }
   psToken->eType = eTokenType;

   psToken->pcValue = (char*)malloc(strlen(pcValue) + 1);
   if (psToken->pcValue == NULL)
   {
      free(psToken);
      fprintf(stderr, "Cannot allocate memory\n");
      return NULL;
   }

   strcpy(psToken->pcValue, pcValue);

   return psToken;
}


/*--------------------------------------------------------------------*/

int lexLine(const char *pcLine, DynArray_T oTokens)

/* Lexically analyze string pcLine.  Populate oTokens with the
   tokens that pcLine contains.  Return 1 (TRUE) if successful, or
   0 (FALSE) otherwise.  In the latter case, oTokens may contain
   tokens that were discovered before the error. The caller owns the
   tokens placed in oTokens. */

/* lexLine() uses a DFA approach.  It "reads" its characters from
   pcLine. */

{
   enum LexState {STATE_START, STATE_IN_NUMBER, STATE_IN_WORD};

   enum LexState eState = STATE_START;

   int iLineIndex = 0;
   int iValueIndex = 0;
   int word_detector = 0;
   char c;
   char q;
   char acValue[MAX_LINE_SIZE];
   char pipe[MAX_LINE_SIZE];
   char in_redir[MAX_LINE_SIZE];
   char out_redir[MAX_LINE_SIZE];
   char background[MAX_LINE_SIZE];
   struct Token *psToken;

   assert(pcLine != NULL);
   assert(oTokens != NULL);
   for (;;)
   {
      /* "Read" the next character from pcLine. */
      c = pcLine[iLineIndex++];

      switch (eState)
      {
         case STATE_START:
            if ((c == '\n') || (c == '\0')){
               return TRUE;
            }
            else if (isdigit(c))
            {
               acValue[iValueIndex++] = c;
               eState = STATE_IN_NUMBER;
            }
            else if (isalpha(c))
            {
               acValue[iValueIndex++] = c;
               eState = STATE_IN_WORD;
            }
            else if (isspace(c))
            {
               eState = STATE_START;
            }    
            else if ((c == '"')||(c == '\'')){
               q = c;
               c = pcLine[iLineIndex++];
               int digit = 0;
               while ((q == '"') ? c!='"' : c!= '\''){
               if (!isdigit(c)){
                  digit = 1;
                  }
               if ((c == '\n')||(c == '\0')){
                  fprintf(stderr, "Invalid: Unmatched quote\n");
                  return FALSE;
               }
               acValue[iValueIndex++] = c;
               c = pcLine[iLineIndex++];
               }
               if (digit == 0){
                  eState = STATE_IN_NUMBER;
               }
               else{
                  eState = STATE_IN_WORD;
               }
               
            }
            else if (c == '|'){
               pipe[0] = c;
               pipe[1] = '\0';
               psToken = makeToken(TOKEN_PIPE, pipe);
               if (psToken == NULL)
               {
               fprintf(stderr, "Cannot allocate memory\n");
               return FALSE;
            }
            if (! DynArray_add(oTokens, psToken))
            {
               fprintf(stderr, "Cannot allocate memory\n");
               return FALSE;
            }
            iValueIndex = 0;
            eState = STATE_START;
        
            }
            else if(c == '<'){
               in_redir[0] = c;
               in_redir[1] = '\0';
               psToken = makeToken(TOKEN_INREDIR, in_redir);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               iValueIndex = 0;
               eState = STATE_START;
               }
            else if(c == '>'){
               out_redir[0] = c;
               out_redir[1] = '\0';
               psToken = makeToken(TOKEN_OUTREDIR, out_redir);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               iValueIndex = 0;
               eState = STATE_START;
               }
            else if(c == '&'){
               background[0] = c;
               background[1] = '\0';
               psToken = makeToken(TOKEN_BACKGROUND, background);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               iValueIndex = 0;
               eState = STATE_START;
               }
            else
            {
               acValue[iValueIndex++] = c;
               eState = STATE_IN_WORD;
            }
            break;

         case STATE_IN_NUMBER:
            if ((c == '\n') || (c == '\0'))
            {
               /* Create a NUMBER token. */
               acValue[iValueIndex] = '\0';
               psToken = makeToken(TOKEN_NUMBER, acValue);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               iValueIndex = 0;
               if(c == '\0'){
                  return TRUE;
               }
               eState = STATE_START;
            }
            else if (isdigit(c))
            {
               acValue[iValueIndex++] = c;
               if (word_detector == 0){
                  eState = STATE_IN_NUMBER;
               }
               else{
                  eState = STATE_IN_WORD;
               }
            }
            else if (isalpha(c))
            {
               acValue[iValueIndex++] = c;
               word_detector = 1;
               eState = STATE_IN_WORD;
            }
            else if (isspace(c))
               {
               /* Create a NUMBER token. */
               acValue[iValueIndex] = '\0';
               psToken = makeToken(TOKEN_NUMBER, acValue);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               iValueIndex = 0;
               eState = STATE_START;
            }
            else if ((c == '"')||(c == '\'')){
               q = c;
               c = pcLine[iLineIndex++];
               int i = 0;
               while ((q == '"') ? c!='"' : c!= '\''){
                  if ((c == '\n')||(c == '\0')){
                     fprintf(stderr, "Invalid: Unmatched quote\n");
                     return FALSE;
               }
               if (!isdigit(c)){
                  i = 1;
               }
               acValue[iValueIndex++] = c;
               c = pcLine[iLineIndex++];
               }

               if (i == 0){
                  eState = STATE_IN_NUMBER;
               }
               else {
                  eState = STATE_IN_WORD;
               }
            }
            else if (c == '|'){
               acValue[iValueIndex] = '\0';
               psToken = makeToken(TOKEN_NUMBER, acValue);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               pipe[0] = c;
               pipe[1] = '\0';
               psToken = makeToken(TOKEN_PIPE, pipe);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               iValueIndex = 0;
               eState = STATE_START;
               }
            else if(c == '<'){
               acValue[iValueIndex] = '\0';
               psToken = makeToken(TOKEN_NUMBER, acValue);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               in_redir[0] = c;
               in_redir[1] = '\0';
               psToken = makeToken(TOKEN_INREDIR, in_redir);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               iValueIndex = 0;
               eState = STATE_START;
               }
            else if(c == '>'){
               acValue[iValueIndex] = '\0';
               psToken = makeToken(TOKEN_NUMBER, acValue);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               out_redir[0] = c;
               out_redir[1] = '\0';
               psToken = makeToken(TOKEN_OUTREDIR, out_redir);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               iValueIndex = 0;
               eState = STATE_START;
               }
            else if(c == '&'){
               acValue[iValueIndex] = '\0';
               psToken = makeToken(TOKEN_NUMBER, acValue);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               background[0] = c;
               background[1] = '\0';
               psToken = makeToken(TOKEN_BACKGROUND, background);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               iValueIndex = 0;
               eState = STATE_START;
               }
            else
            {
               acValue[iValueIndex++] = c;
               eState = STATE_IN_WORD;
            }
            break;

         case STATE_IN_WORD:
            if ((c == '\n') || (c == '\0'))
            {
               /* Create a WORD token. */
               acValue[iValueIndex] = '\0';
               psToken = makeToken(TOKEN_WORD, acValue);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               iValueIndex = 0;
               if(c == '\0'){
                  return TRUE;
               }
               eState = STATE_START;

               
            }
            else if (isalpha(c))
            {
               acValue[iValueIndex++] = c;
               word_detector = 1;
               eState = STATE_IN_WORD;
            }
            else if (isdigit(c))
            {
               acValue[iValueIndex++] = c;
               if (word_detector == 0){
                  eState = STATE_IN_NUMBER;
               }
               else{
                  eState = STATE_IN_WORD;
               }
            }
            else if (isspace(c))
            {
               /* Create a WORD token. */
               acValue[iValueIndex] = '\0';
               psToken = makeToken(TOKEN_WORD, acValue);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               iValueIndex = 0;
               eState = STATE_START;
            }
             else if ((c == '"')||(c == '\'')){
               q = c;
               c = pcLine[iLineIndex++];
               int i = 0;
               while ((q == '"') ? c!='"' : c!= '\''){
                  if ((c == '\n')||(c == '\0')){
                     fprintf(stderr, "Invalid: Unmatched quote\n");
                     return FALSE;
               }
               if (!isdigit(c)){
                  i = 1;
               }
               acValue[iValueIndex++] = c;
               c = pcLine[iLineIndex++];
               }

               if (i == 0){
                  eState = STATE_IN_NUMBER;
               }
               else {
                  eState = STATE_IN_WORD;
               }
            }
             else if (c == '|'){
               acValue[iValueIndex] = '\0';
               psToken = makeToken(TOKEN_WORD, acValue);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               iValueIndex = 0;
               pipe[0] = c;
               pipe[1] = '\0';
               psToken = makeToken(TOKEN_PIPE, pipe);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               iValueIndex = 0;
               eState = STATE_START;
             }
            else if(c == '<'){
               acValue[iValueIndex] = '\0';
               psToken = makeToken(TOKEN_WORD, acValue);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               in_redir[0] = c;
               in_redir[1] = '\0';
               psToken = makeToken(TOKEN_INREDIR, in_redir);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               iValueIndex = 0;
               eState = STATE_START;
               }
            else if(c == '>'){
               acValue[iValueIndex] = '\0';
               psToken = makeToken(TOKEN_WORD, acValue);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               out_redir[0] = c;
               out_redir[1] = '\0';
               psToken = makeToken(TOKEN_OUTREDIR, out_redir);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               iValueIndex = 0;
               eState = STATE_START;
               }
             else if(c == '&'){
               acValue[iValueIndex] = '\0';
               psToken = makeToken(TOKEN_WORD, acValue);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }

               background[0] = c;
               background[1] = '\0';
               psToken = makeToken(TOKEN_BACKGROUND, background);
               if (psToken == NULL)
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               if (! DynArray_add(oTokens, psToken))
               {
                  fprintf(stderr, "Cannot allocate memory\n");
                  return FALSE;
               }
               iValueIndex = 0;
               eState = STATE_START;
               }
            else
            {
               acValue[iValueIndex++] = c;
               eState = STATE_IN_WORD;
            }
            break;
         default:
            assert(FALSE);
      }
   }
}

/*--------------------------------------------------------------------*/
/*
int main(void) 

 Read a line from stdin, and write to stdout each number and word
   that it contains.  Repeat until EOF.  Return 0 iff successful. 

{
   char acLine[MAX_LINE_SIZE];
   DynArray_T oTokens;
   int iSuccessful;

  
   printf("------------------------------------\n");
   while (fgets(acLine, MAX_LINE_SIZE, stdin) != NULL)
   {
      oTokens = DynArray_new(0);
      if (oTokens == NULL)
      {
         fprintf(stderr, "Cannot allocate memory\n");
         exit(EXIT_FAILURE);
      }

      iSuccessful = lexLine(acLine, oTokens);
      if (iSuccessful)
      {
          DynArray_map(oTokens, printToken, NULL);
          printf("\n");

      }
      printf("------------------------------------\n");

      DynArray_map(oTokens, freeToken, NULL);
      DynArray_free(oTokens);
   }

   return 0;
} */