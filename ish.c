

#include "dynarray.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include "dfa.h"
#include "syntactic.h"
#include "ish.h"
#define _D_DEFAULT_SOURCE
#define _D_GNU_SOURCE

static int r = 0;
//array which contains buit-in commands
char* builtin_cmds[4] = {"setenv", "cd" ,"unsetenv", "exit"};

/*check if there is no special character*/
int validate(DynArray_T Lexout){
    int index = 0;
    while (index < (DynArray_getLength(Lexout))){
        struct Token* ptr = DynArray_get(Lexout, index);
        if( ptr->eType != TOKEN_WORD){
            return FALSE;
        }
        index++;
    }
    return TRUE;
}
/* this function checks the validity of the given paremetres 
for setenv and implements it*/
int execute_setenv(DynArray_T Synout, char* ish){
    token_types* cmd = DynArray_get(Synout, 0);
    DynArray_T args = cmd->normal_tokens;
    char* first_arg = DynArray_get(args, 0);
    if (DynArray_getLength(args) == 1){
        if(setenv(first_arg, "", 1) == -1){
            perror(ish);
            return FALSE;
        }
    }
    else if(DynArray_getLength(args) == 2){
        char* second_arg = DynArray_get(args, 1);
        if(setenv(first_arg,second_arg, 1) == -1){
            perror(ish);
            return FALSE;
        }
    }
    else{
         fprintf(stderr, "%s: setenv takes one or two parameters\n", ish);
         fflush(stderr);
        return FALSE;
    }
    return TRUE;
}
/* this function checks the validity of the given paremetres 
for unsetenv and implements it*/
int execute_unsetenv(DynArray_T Synout, char* ish){
    token_types* cmd = DynArray_get(Synout, 0);
    DynArray_T args = cmd->normal_tokens;
    const char* env = DynArray_get(args, 0);
    if(DynArray_getLength(args) !=1 ){
        fprintf(stderr, "%s: unsetenv takes one parameter\n", ish);
        return FALSE;
    } 
    if(unsetenv(env) == -1){
        perror(ish);
        return FALSE;
    }
    return TRUE;
}
/* this function checks the validity of the given paremetres 
for cd and implements it*/
int execute_cd(DynArray_T Synout, char* ish){
    token_types* cmd = DynArray_get(Synout, 0);
    DynArray_T args = cmd->normal_tokens;
    if(args == NULL){
        if(chdir(getenv("HOME")) == -1){
            perror(ish);
           return FALSE;
           }
       }
    else if(DynArray_getLength(args) == 1){
        const char* env = DynArray_get(args, 0);
        if(chdir(env) == -1){
            perror(ish);
           return FALSE;
           }
       }
    else{
        fprintf(stderr, "%s: cd takes one parameter\n", ish);
        return FALSE;
    }
    return TRUE;
}
/*this function takes the outputs of the lecical and syntactic analysis
 to execute buitin commands using the above defined functions.*/
static int builtins(DynArray_T Lexout, DynArray_T Synout, char* ish){
    assert(Synout);
    token_types* cmd = DynArray_get(Synout, 0);
    char* command = cmd->command_name;
    assert(command != NULL);
    if (!validate(Lexout)){
        return FALSE;
    }

    if(strcmp(command,"setenv") == 0){
        execute_setenv(Synout, ish);
    }
    else if(strcmp(command,"unsetenv") == 0){
        execute_unsetenv(Synout, ish);
    }
    else if(strcmp(command,"cd") == 0){
        execute_cd(Synout, ish);
    }
    else if (strcmp(command,"exit") == 0){
        token_types* cmd = DynArray_get(Synout, 0);
        DynArray_T args = NULL;
        args = cmd->normal_tokens;
        if(args != NULL){
            fprintf(stderr, "%s: exit does not take any parameters\n", ish);
            fflush(NULL);
            return FALSE;
        }
        fflush(NULL);
        printf("\n");
        exit(0);
    }
    return TRUE;
    }

static int execute_redirect_In(const char* file_in, char* ish){
    assert(file_in);
    int fd;
    if((fd = open(file_in, O_RDONLY)) == -1){
        perror(ish);
        return FALSE;
    }
    if ((dup2(fd,0)) ==-1 ){
        perror(ish);
        fflush(NULL);
        if(close(fd)==-1){
          perror(ish); 
          fflush(NULL);
        }
        return FALSE;
    }
    if(close(fd)==-1){
        perror(ish);
        fflush(NULL);
        return FALSE;
    }

    return TRUE;
}

static int execute_redirect_Out(const char* file_out, char* ish){
    assert(file_out);
    int fd;
    if((fd = creat(file_out, 0600)) == -1){
        perror(ish);
        fflush(NULL);
        return FALSE;
    }
    if ((dup2(fd,1)) == -1 ){
        perror(ish);
        fflush(NULL);
        if(close(fd)==-1){
          perror(ish);
          fflush(NULL); 
        }
        return FALSE;
    }
    if(close(fd) == -1){
        perror(ish);
        fflush(NULL);
        return FALSE;
    }
    return TRUE;
}

void redirection(DynArray_T Synout, int index, char* ish){
    token_types* command_line = DynArray_get(Synout, index);
    char* red_in;
    char* red_out;
    red_in = command_line->redir_in_token;
    red_out = command_line->redir_out_token;
    if(red_in != NULL){
        execute_redirect_In(red_in, ish);
    }
    else if(red_out != NULL){
        execute_redirect_Out(red_out, ish);
    }
    return;
}


struct Table *Table_create(void) {
struct Table *t;
t = (struct Table *) malloc(sizeof(struct Table));
if(t == NULL){
    free(t);
    return FALSE;
}
t->first = NULL;
return t;
}

void Table_add(struct Table *t, const char *key, pid_t value)
{
struct Node *p = (struct Node*)malloc(sizeof(struct Node));
if(p== NULL){
    free(p);
    return;
}
p->key = key;
p->value = value;
p->next = t->first;
t->first = p;
}
            
void Table_free(struct Table *t) {
struct Node *p;
struct Node *nextp;
for (p = t->first; p != NULL; p = nextp) {
nextp = p->next;
free(p);
}
free(t);
}



static int execute_single_command(DynArray_T Synout, char* ish){
    assert(Synout != NULL);
    char* const* command_array;
    pid_t pid;
    struct Node *q;
    token_types* command_line = DynArray_get(Synout, 0);
    const char* std_in = command_line->redir_in_token;
    const char* std_out = command_line->redir_out_token;
    const char* background = command_line->background_token;
    DynArray_T arg = command_line->normal_tokens;
    void* command_name = command_line->command_name;
    
    if(arg != NULL){
        if(DynArray_add(arg, NULL) == FALSE){
            fprintf(stderr, "%s: cannot allocate memory\n", ish);
            fflush(NULL);
            return(FALSE);
        }
    }
    else {
        arg = DynArray_new(0);
    }
    if(DynArray_addAt(arg, 0, command_name) == FALSE){
        fprintf(stderr, "%s: cannot allocate memory\n", ish);
        fflush(NULL);
        return(FALSE);
    }
    command_array = (char * const*)(arg)->ppvArray;
    
    fflush(NULL);
    pid = fork();
    
    if(pid < 0){
        fprintf(stderr, "%s: Unable to create child process\n", ish);
        fflush(NULL);
        return(FALSE);
    }
    if (pid != 0){ //in parent
        if((background == NULL) &&(strcmp(command_array[0], "fg") != 0)){
        pid = wait(NULL);
        }
        else if(strcmp(command_array[0], "fg")==0){
            
            q = p->first;
            pid_t last_pid = q->value;
            kill(pid, SIGKILL);
            waitpid(last_pid, NULL, 0);
            printf("[%d] Latest background process is running\n", last_pid);
            fflush(NULL);
        }
        else{
            printf("[%d] Background process is created\n", pid);
            fflush(NULL);
            Table_add(p, "pid_key", pid);
        }
    }
    else if(pid == 0){ //in child
        signal(SIGINT, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        if((std_in !=NULL)||(std_out != NULL)){
            redirection(Synout, 0, ish);
            }
    
    fflush(stdout);
    execvp(command_array[0], command_array);
    fprintf(stderr, "%s: No such file or directory\n",command_array[0]);
    exit(EXIT_FAILURE);
    }
    return TRUE;
}


static int execute_pipe(DynArray_T Synout, char* ish){
    assert(Synout != NULL);
    pid_t pid;
    int i;
    int length = (Synout->iLength);
    int num_pipes = length-1;
    int pipe_fd[2*num_pipes];
    
    
    for (i = 0; i<num_pipes; i++){
        if(pipe(pipe_fd + 2*i) == -1){
            perror("pipe");
            exit(EXIT_FAILURE);
    }
    }
    int num_commands = 0;
    int fd = 1;
while(num_commands < length) {
    char* const* command_array;
    token_types* command_line = (token_types *)DynArray_get(Synout, num_commands);
    const char* red_in = command_line->redir_in_token;
    const char* red_out = command_line->redir_out_token;
    DynArray_T arg = command_line->normal_tokens;
    void* command_name = command_line->command_name;

    if(arg == NULL){
         arg = DynArray_new(0);
    }
        
    if(DynArray_add(arg, NULL) == FALSE){
            fprintf(stderr, "%s: cannot allocate memory\n", ish);
            fflush(NULL);
            exit(EXIT_FAILURE);
        }
    if(DynArray_addAt(arg, 0, command_name) == FALSE){
        fprintf(stderr, "%s: cannot allocate memory\n", ish);
        fflush(NULL);
        exit(EXIT_FAILURE);
    }
    command_array = (char * const*)arg->ppvArray;
    fflush(NULL);
    pid = fork();
    if(pid < 0){
        perror(ish);
        fflush(NULL);
        exit(EXIT_FAILURE);
    }
    if(pid == 0){ //in child

        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        if(num_commands == 0){
            if(red_in != NULL){
                redirection(Synout, num_commands, ish);
            }
            if(dup2(pipe_fd[1], 1) < 0){
                perror(ish);
                fflush(NULL);
                exit(EXIT_FAILURE);
            }
            for (int i = 0; i < 2*num_pipes; i++){
                if(close(pipe_fd[i])==-1){
                perror(ish);
                fflush(NULL);
                exit(EXIT_FAILURE);
            }
            }
            execvp(command_array[0], command_array);
            perror(command_array[0]);
            fflush(NULL);
            exit(EXIT_FAILURE);
        } 
        else if(num_commands == num_pipes){
            if(red_out != NULL){
                redirection(Synout, num_commands, ish);
            }
            if(dup2(pipe_fd[2*num_commands-2], 0)<0){
                perror(ish);
                fflush(NULL);
                exit(EXIT_FAILURE);
            }
            for (int i = 0; i < 2*num_pipes; i++){
                if(close(pipe_fd[i])==-1){
                perror(ish);
                fflush(NULL);
                exit(EXIT_FAILURE);
            }
            }

            execvp(command_array[0], command_array);
            perror(command_array[0]);
            fflush(NULL);
            exit(EXIT_FAILURE);
        }
        else if((num_commands != 0)&&(num_commands != num_pipes)){
            if(dup2(pipe_fd[2*num_commands-2], 0)==-1){
                perror(ish);
                fflush(NULL);
                exit(EXIT_FAILURE);
            }
             if(dup2(pipe_fd[2*num_commands + 1], 1) == -1){
                perror(ish);
                fflush(NULL);
                exit(EXIT_FAILURE);
            }
            for (int i = 0; i < 2*num_pipes; i++){
                if(close(pipe_fd[i])==-1){
                perror(ish);
                fflush(NULL);
                exit(EXIT_FAILURE);
            }
            }
            execvp(command_array[0], command_array);
            perror(command_array[0]);
            fflush(NULL);
            exit(EXIT_FAILURE);
        }
        
    }
    num_commands++;
    fd +=2;
}
    for (i = 0; i < 2*num_pipes; i++){
            if(close(pipe_fd[i])==-1){
            perror(ish);
            fflush(NULL);
            exit(EXIT_FAILURE);
            }
            }

    for(i=0; i<num_pipes + 1 ; i++){
        wait(NULL);
    }
   return TRUE;

}

static void reset_handler(int isig){
    r = 0;
}
static void my_sig_quit_handler(int isig){
    r++;
    if(r>1){
        exit(0);
    }
    else if(r == 1){
    printf("\nType Ctrl-\\ again within 5 seconds to exit.\n");
	fflush(stdout);
	alarm(5);
    }
}

/*adapted from lecture note*/

FILE *psFile;
static void cleanup(int iSig){
    fclose(psFile);
    remove("temp.txt");
    exit(0);
}
int main(){
    int R;
    sigset_t s;
    
    sigaddset(&s,SIGALRM);
    sigaddset(&s,SIGQUIT);
    sigaddset(&s,SIGINT);
    R = sigprocmask(SIG_UNBLOCK,&s,NULL);
    assert(R == 0);
	
	signal(SIGINT,SIG_IGN);
	signal(SIGQUIT, my_sig_quit_handler);
	signal(SIGALRM,reset_handler);

    char acLine[MAX_LINE_SIZE];
    p = Table_create();
    DynArray_T oTokens;
    DynArray_T new_tokens;
    char* synt = "./ish";
    char* ish = "./ish";

    char* home;
    char* file_name;
    char* finish;
    psFile = fopen("temp.txt", "w");
    if(psFile != NULL){
        printf("%s", acLine);
		fflush(NULL);
    }
   
    size_t size;
    FILE *ptr = NULL;
    home = getenv("HOME");
    if(home != NULL){
        size = strlen("/.ishrc") + strlen(home) + 1;
        file_name = calloc(size, sizeof(char));
        if(file_name == NULL){
            fprintf(stderr, "cannot allocate memory\n");
            free(file_name);
        }
    }
  
    if(file_name != NULL){
        strcpy(file_name, home);
        strcat(file_name, "/.ishrc");
        ptr = fopen(file_name, "r");
        free(file_name);
        }
        fflush(NULL);
    printf("%% ");
	fflush(stdout);

while ((ptr && (finish = fgets(acLine, MAX_LINE_SIZE, ptr)))||(fgets(acLine, MAX_LINE_SIZE, stdin) != NULL)){
    if(!finish && ptr){
        fclose(ptr);
        ptr = NULL;
    }
    if(ptr != NULL){
        printf("%s", acLine);
        fflush(NULL);
    }
    oTokens = DynArray_new(0);
    int Lexout = lexLine(acLine, oTokens);
    if(oTokens == NULL){
        printf("%% ");
        fflush(NULL);
        continue;
    }
    if(Lexout == FALSE){
        printf("%% ");
        fflush(NULL);
        continue;
    }
    new_tokens = DynArray_new(0);
    int Synout = synAnalysis(oTokens,new_tokens, synt);
    if(Synout == FALSE){
        printf("%% ");
        fflush(NULL);
        continue;
    }
    if(new_tokens == NULL){
        printf("%% ");
        fflush(NULL);
        continue;
    }
    int builtin = 0;
    token_types* command_line = DynArray_get(new_tokens, 0);
    const char* cmd = command_line->command_name;
    for (int i = 0; i < 4; i++){
        if(strcmp(cmd, builtin_cmds[i]) == 0){
            builtin = 1;
        }
    }
    int index = 0;
    int indicator = 0;
    int length = DynArray_getLength(oTokens);
    while (index < length){
        struct Token* token_pointer = DynArray_get(oTokens, index);
        if( token_pointer->eType == TOKEN_PIPE){
            indicator = 1;
        }
        index++;
    }
    
    if(indicator == 1){
        execute_pipe(new_tokens, ish);
        printf("%% ");
        fflush(stdout);
    }
    else if(builtin == 0){
        execute_single_command(new_tokens, ish);
        printf("%% ");
        fflush(stdout);
    }
    else if( builtin == 1){
        builtins(oTokens, new_tokens, ish);
        printf("%% ");
        fflush(stdout);
    }
    memset(acLine, 0, MAX_LINE_SIZE);
}
    printf("\n");
    cleanup(0);
    if(ptr != NULL){
        fclose(ptr);
    }

    return TRUE;
}