
#ifndef SYNTACTIC_INCLUDED
#define SYNTACTIC_INCLUDED
#include "dynarray.h"

int synAnalysis(DynArray_T oTokens, DynArray_T new_token, char *synt);

void print_args(void *pvItem, void *pvExtra);
typedef struct token_type
{
    char* command_name;
    char* pipe_token;
    char* redir_in_token;
    char* redir_out_token;
    char* background_token;
    DynArray_T normal_tokens;

}token_types;

#endif