
//syntactic.c/
#include "dynarray.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "dfa.h"
#include "syntactic.h"

#define _DEBUG


token_types* save_mycommands(char* command_name, char* pipe_token, char* redir_in_token, char* redir_out_token, char* background_token, DynArray_T normal_token){
   
    token_types *mycommands = calloc(1, sizeof(token_types));
    if(mycommands == NULL){
        free(mycommands);
        fprintf(stderr, "Cannot allocate memory\n");
        exit(EXIT_FAILURE);
    }
   
     
    if(command_name != NULL){
        mycommands->command_name = calloc(1, strlen(command_name)+1);
        if(mycommands->command_name == NULL){
        free(mycommands->command_name);
        fprintf(stderr, "Cannot allocate memory\n");
        exit(EXIT_FAILURE);
    }
        strcpy(mycommands->command_name, command_name);
        free(command_name);
    }
    else{
        mycommands->command_name = NULL;
    }
  
    if(redir_in_token != NULL){
        mycommands->redir_in_token = calloc(1, strlen(redir_in_token)+1);
        if(mycommands->redir_in_token == NULL){
        free(mycommands->redir_in_token);
        fprintf(stderr, "Cannot allocate memory\n");
        exit(EXIT_FAILURE);
    }
        strcpy(mycommands->redir_in_token, redir_in_token);
        free(redir_in_token);
    }
    else{
        mycommands->redir_in_token = NULL;
    }
    
    if(redir_out_token != NULL){
        mycommands->redir_out_token = calloc(1, strlen(redir_out_token)+1);
        if(mycommands->redir_out_token == NULL){
        free(mycommands->redir_out_token);
        fprintf(stderr, "Cannot allocate memory\n");
        exit(EXIT_FAILURE);
    }
        strcpy(mycommands->redir_out_token, redir_out_token);
        free(redir_out_token);
    }
    else{
        mycommands->redir_out_token = NULL;
    }
    if(background_token != NULL){
        mycommands->background_token = calloc(1, strlen(background_token)+1);
        if(mycommands->background_token == NULL){
        free(mycommands->background_token);
        fprintf(stderr, "Cannot allocate memory\n");
        exit(EXIT_FAILURE);
    }
        strcpy(mycommands->background_token, background_token);
        free(background_token);
    }
    else{
        mycommands->background_token = NULL;
    }
    mycommands->pipe_token = NULL;
    mycommands->normal_tokens = normal_token;
    return mycommands;
}

void print_args(void *pvItem, void *pvExtra)
    {
        char* arg_ptr = (char *)pvItem;
        printf("%s\n", arg_ptr);
    }

void print(void *pvItem, void *pvExtra)

/* Print token pvItem to stdout.  pvExtra is
   unused. */

{
   token_types *t = (token_types*) pvItem;
    printf("Command: %s\n", t->command_name);
    printf("red_in: %s\n", t->redir_in_token); 
    printf("args:\n");
    if(t->normal_tokens != NULL){
    DynArray_map(t->normal_tokens, print_args, NULL);
   }
    printf("bkgrnd: %s\n", t->background_token);
    printf("red_out: %s\n", t->redir_out_token);

   
}
int synAnalysis(DynArray_T oTokens, DynArray_T new_token, char *synt){
    struct Token *psToken;
    int index = 0;
    char* command = NULL;
    char* RedIn = NULL;
    char* RedOut = NULL;
    char* Pipe = NULL;
    char* background = NULL;
    DynArray_T args = NULL;

    enum SynState{STATE_START, STATE_WORD, STATE_PIPE, STATE_RED_IN, STATE_RED_OUT, STATE_BACKGROUND};
    enum SynState eState = STATE_START;
    token_types *cmd;
    while(index != DynArray_getLength(oTokens))
    {
    psToken = DynArray_get(oTokens, index);
    switch (eState) {
        case STATE_START:
            if (psToken->eType == TOKEN_WORD){
                command = calloc(1, strlen(psToken->pcValue)+1);
                if(command == NULL){
                    free(command);
                    fprintf(stderr, "Cannot allocate memory\n");
                    exit(EXIT_FAILURE);
                }
                strcpy(command, psToken->pcValue);
                index++;
                eState = STATE_WORD;
            }
            else if(psToken->eType == TOKEN_NUMBER){
                command = calloc(1, strlen(psToken->pcValue)+1);
                if(command == NULL){
                    free(command);
                    fprintf(stderr, "Cannot allocate memory\n");
                    exit(EXIT_FAILURE);
                }
                strcpy(command, psToken->pcValue);
                index++;
                eState = STATE_WORD;
            }
            else {
                fprintf(stderr, "%s: Missing command name\n", synt);
                printf("%% \n");
                fflush(NULL);
                exit(EXIT_FAILURE);
            }
            break;
        case STATE_WORD:
            if ((psToken->eType == TOKEN_WORD) || (psToken->eType == TOKEN_NUMBER)){
                if(args == NULL){
                args = DynArray_new(0);
                }
                DynArray_add(args, psToken->pcValue);
                index++;
                eState = STATE_WORD;
            }
               
            else if(psToken->eType == TOKEN_BACKGROUND){
                background = calloc(1, strlen(psToken->pcValue)+1);
                 if(background == NULL){
                    free(background);
                    fprintf(stderr, "Cannot allocate memory\n");
                    printf("%% \n");
                    fflush(NULL);
                    exit(EXIT_FAILURE);
                }
                strcpy(background,psToken->pcValue);
                index++;
                eState = STATE_BACKGROUND;
            }
            else if(psToken->eType == TOKEN_PIPE){
                Pipe = calloc(1, strlen(psToken->pcValue)+1);
                 if(Pipe == NULL){
                    free(Pipe);
                    fprintf(stderr, "Cannot allocate memory\n");
                    printf("%% \n");
                    fflush(NULL);
                    exit(EXIT_FAILURE);
                }
                strcpy(Pipe,psToken->pcValue);
                if(RedOut != NULL){
                fprintf(stderr, "%s: Multiple redirection of standard out\n", synt);
                printf("%% \n");
                fflush(NULL);
                exit(EXIT_FAILURE);
                }
                index++;
                eState = STATE_PIPE;
            }
            else if(psToken->eType == TOKEN_INREDIR){
                if((RedIn != NULL)||(Pipe != NULL)){
                fprintf(stderr, "%s: Multiple redirection of standard input\n", synt);
                printf("%% \n");
                fflush(NULL);
                exit(EXIT_FAILURE);
                }
                index++;
                eState = STATE_RED_IN;
            }
            else if(psToken->eType == TOKEN_OUTREDIR){
                if(RedOut != NULL){
                fprintf(stderr, "%s: Multiple redirection of standard out\n", synt);
                printf("%% \n");
                fflush(NULL);
                exit(EXIT_FAILURE);
                }
                index++;
                eState = STATE_RED_OUT;
            }
           
            break;
        case STATE_RED_IN:
            if(psToken->eType == TOKEN_INREDIR){
                fprintf(stderr, "%s: Multiple redirection of standard input\n", synt);
                printf("%% \n");
                fflush(NULL);
                exit(EXIT_FAILURE);
                }
            else if (psToken->eType == TOKEN_WORD){
                RedIn = calloc(1, strlen(psToken->pcValue)+1);
                 if(RedIn == NULL){
                    free(RedIn);
                    fprintf(stderr, "Cannot allocate memory\n");
                    exit(EXIT_FAILURE);
                }
                strcpy(RedIn,psToken->pcValue);
                index++;
                eState = STATE_WORD;
            }
            else if(psToken->eType == TOKEN_BACKGROUND){
                fprintf(stderr, "%s: Missing command name\n", synt);
                return(EXIT_FAILURE);
            }
            else if(psToken->eType == TOKEN_PIPE){
                fprintf(stderr, "%s: Standard input redirection without file name\n", synt);
                printf("%% \n");
                fflush(NULL);
                return(EXIT_FAILURE);
            }
            else if(psToken->eType == TOKEN_OUTREDIR){
                fprintf(stderr, "%s: Missing command name\n", synt);
                printf("%% \n");
                fflush(NULL);
                exit(EXIT_FAILURE);
                }
           
            break;
        case STATE_RED_OUT:
            if(psToken->eType == TOKEN_OUTREDIR){
                fprintf(stderr, "%s: Multiple redirection of standard out\n", synt);
                printf("%% \n");
                fflush(NULL);
                exit(EXIT_FAILURE);
                }
            else if (psToken->eType == TOKEN_WORD){
                RedOut = calloc(1, strlen(psToken->pcValue)+1);
                 if(RedOut == NULL){
                    free(RedOut);
                    fprintf(stderr, "Cannot allocate memory\n");
                    printf("%% \n");
                    fflush(NULL);
                    exit(EXIT_FAILURE);
                }
                strcpy(RedOut, psToken->pcValue);
                index++;
                eState = STATE_WORD;
            }
            else if(psToken->eType == TOKEN_BACKGROUND){
                fprintf(stderr, "%s: Missing command name\n", synt);
                printf("%% \n");
                fflush(NULL);
                exit(EXIT_FAILURE);
            }
            else if(psToken->eType == TOKEN_PIPE){
                fprintf(stderr, "%s: Standard output redirection without file name\n", synt);
                printf("%% \n");
                fflush(NULL);
                exit(EXIT_FAILURE);
            }
            else if(psToken->eType == TOKEN_INREDIR){
                fprintf(stderr, "%s: Missing command name\n", synt);
                printf("%% \n");
                fflush(NULL);
                exit(EXIT_FAILURE);
            }
            break;
        case STATE_BACKGROUND:
            if(psToken->eType == TOKEN_BACKGROUND){
                fprintf(stderr, "%s: Multiple background\n", synt);
                printf("%% \n");
                fflush(NULL);
                exit(EXIT_FAILURE);
                }

            else if (psToken->eType == TOKEN_WORD){
                background = calloc(1, strlen(psToken->pcValue)+1);
                 if(background == NULL){
                    free(background);
                    fprintf(stderr, "Cannot allocate memory\n");
                    printf("%% \n");
                    fflush(NULL);
                    exit(EXIT_FAILURE);
                }
                strcpy(background, psToken->pcValue);
                index++;
                eState = STATE_WORD;
            }
            else if(psToken->eType == TOKEN_PIPE){
                 fprintf(stderr, "%s: Missing command name\n", synt);
                 printf("%% \n");
                 fflush(NULL);
                exit(EXIT_FAILURE);
            }
            else if(psToken->eType == TOKEN_INREDIR){
                fprintf(stderr, "%s: Missing command name\n", synt);
                printf("%% \n");
                fflush(NULL);
                exit(EXIT_FAILURE);
            }
            else if(psToken->eType == TOKEN_OUTREDIR){
                 fprintf(stderr, "%s: Missing command name\n", synt);
                 printf("%% \n");
                fflush(NULL);
                exit(EXIT_FAILURE);
            }
            break;
         case STATE_PIPE:
            if(psToken->eType == TOKEN_PIPE){
                fprintf(stderr, "%s: Missing command name\n", synt);
                printf("%% \n");
                fflush(NULL);
                exit(EXIT_FAILURE);
                }

            else if (psToken->eType == TOKEN_WORD){
                cmd = save_mycommands(command, Pipe, RedIn, RedOut, background, args);
                DynArray_add(new_token, cmd);
                args = NULL;
                command = NULL;
                RedIn = NULL;
                RedOut = NULL;
                background = NULL;
                eState = STATE_START;
            }
            else if(psToken->eType == TOKEN_BACKGROUND){
                fprintf(stderr, "%s: Missing command name\n", synt);
                printf("%% \n");
                fflush(NULL);
                exit(EXIT_FAILURE);
            }
            else if(psToken->eType == TOKEN_INREDIR){
                fprintf(stderr, "%s: Missing command name\n",synt);
                printf("%% \n");
                fflush(NULL);
                exit(EXIT_FAILURE);
            }
            else if(psToken->eType == TOKEN_OUTREDIR){
                fprintf(stderr, "%s: Missing command name\n", synt);
                printf("%% \n");
                fflush(NULL);
                exit(EXIT_FAILURE);
            }
            break;
        default:
            return(FALSE);
    }
    }
    if (psToken->eType == TOKEN_PIPE){
        fprintf(stderr, "%s: Missing command name\n", synt);
        printf("%% \n");
        fflush(NULL);
        exit(EXIT_FAILURE);
    }
    if (psToken->eType == TOKEN_INREDIR){
        fprintf(stderr, "%s: Standard input redirection without file name\n", synt);
        printf("%% \n");
        fflush(NULL);
        exit(EXIT_FAILURE);
    }
    if (psToken->eType == TOKEN_OUTREDIR){
        fprintf(stderr, "%s: Standard output redirection without file name\n", synt);
        printf("%% \n");
        fflush(NULL);
        exit(EXIT_FAILURE);
    }
    if((psToken->eType != TOKEN_BACKGROUND)&&(background != NULL)){
        fprintf(stderr, "%s: Invalid use of background\n", synt);
        printf("%% \n");
        fflush(NULL);
        exit(EXIT_FAILURE);
    }
    cmd = save_mycommands(command, Pipe, RedIn, RedOut,background, args);
    DynArray_add(new_token, cmd);

    
    return TRUE;
}
/*
int main(void) 

Read a line from stdin, and write to stdout each number and word
   that it contains.  Repeat until EOF.  Return 0 iff successful.

{
   char acLine[MAX_LINE_SIZE];
   DynArray_T oTokens;
    DynArray_T new_tokens;
   int iSuccessful; //, successful;
    char* synt = "ish";
  
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
        new_tokens = DynArray_new(0);
        synAnalysis(oTokens, new_tokens, synt);
    
        DynArray_map(new_tokens, print, NULL);
        printf("\n");

        DynArray_map(new_tokens, freeToken, NULL);
        DynArray_free(new_tokens);

      }
      printf("------------------------------------\n");

      DynArray_map(oTokens, freeToken, NULL);
      DynArray_free(oTokens);
   }

   return 0;
} */

