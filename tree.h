#import "hashtable.h"

struct tree;
typedef struct tree TREE;

typedef void *(GET_FUNCTION)(void *buffer, unsigned long position);

typedef int (EQUALITY_FUNCTION)(void *buffer, unsigned long first, unsigned long last);

typedef char *(TO_STRING_FUNCTION)(void *buffer, unsigned long first, unsigned long last);

typedef struct string {
    void *buffer;
    unsigned long buffer_length;
    GET_FUNCTION *get;
    EQUALITY_FUNCTION *equals;
    TO_STRING_FUNCTION *to_string;
} STRING;

TREE *create_tree(EQUALS_FUNCTION *equals, HASH_FUNCTION *hash);

void print_tree(TREE *tree);
void add_string(TREE *tree, STRING *string);