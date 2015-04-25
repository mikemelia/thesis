#include "hashtable.h"
#include "string.h"

struct tree;
static const int FALSE = 0;
static const int TRUE = 1;
typedef struct tree TREE;

TREE *create_tree(EQUALS_FUNCTION *equals, HASH_FUNCTION *hash);

void print_tree(TREE *tree);

void add_string(TREE *tree, STRING *string);

int num_positions_matching(TREE *tree, char *pattern);

long total_number_of_puts(TREE *tree);
long total_number_of_gets(TREE *tree);
long total_number_of_comparisons(TREE *tree);
long number_of_nodes(TREE *tree);
