
#ifndef ISH_INCLUDED
#define ISH_INCLUDED


char *exe = "./ish";
static int builtins(DynArray_T Lexout, DynArray_T Synout, char* ish);
struct Node {
const char *key;
int value;
struct Node *next;
};

struct Table {
struct Node *first;
};
struct Table *Table_create(void);
struct Table *p;
void Table_add(struct Table *t, const char *key, pid_t value);
void Table_free(struct Table *t);
#endif