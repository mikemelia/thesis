#include "allocation.h"
#include "hashtable.h"
#include "debug.h"
#include "tree.h"
#include <stdlib.h>
#include <string.h>

typedef struct edge {
    int start;
    int *end;
} EDGE;

typedef struct node {
    EDGE *edge;
    HASH_TABLE *children;
} NODE;

typedef struct context {
    int *current_end_position;
    NODE *active_node;
    char active_edge;
    int active_length;
    int remainder;
} CONTEXT;

typedef struct printable {
    NODE *node;
    struct printable *next;
} PRINTABLE;

struct tree {
    EQUALS_FUNCTION *equals;
    HASH_FUNCTION *hash;
    NODE *root;
    CONTEXT *context;
    STRING *string;
    PRINTABLE *printable;
};

void print_node(NODE *node, TREE *tree) {
    int start = node->edge->start;
    int end = *node->edge->end;
    char* label = tree->string->to_string(tree->string->buffer, start, end);
    log_info("NODE: edge %s from %d to %d", label, start, end);
}

void print_context(TREE *tree) {
    CONTEXT *context = tree->context;
    log_info("CONTEXT: active-edge = %c, active_length = %d, remainder = %d", context->active_edge, context->active_length, context->remainder);
}

void print_tree(TREE *tree) {
    PRINTABLE *printable = tree->printable;
    while (printable) {
        NODE *node = printable->node;
        print_node(node, tree);
        printable = printable->next;
    }
    print_context(tree);
}

static HASH_TABLE *create_children(TREE *tree) {
    return create_hash_table(tree->equals, tree->hash, 26);
}

static CONTEXT *initialise_context(TREE *tree) {
    CONTEXT *context = reserve(sizeof(CONTEXT));
    context->active_node = tree->root;
    context->active_edge = 0;
    context->active_length = 0;
    context->remainder = 0;
    context->current_end_position = reserve(sizeof(int));
    *context->current_end_position = -1;
    return context;
}

static TREE *create_initial_implicit_tree(EQUALS_FUNCTION *equals, HASH_FUNCTION *hash) {
    TREE *tree = reserve(sizeof(TREE));
    tree->equals = equals;
    tree->hash = hash;
    tree->root = reserve(sizeof(NODE));
    tree->string = reserve(sizeof(STRING));
    tree->root->edge = NULL;
    tree->root->children = create_children(tree);
    tree->context = initialise_context(tree);
    return tree;
}

static NODE *createNode(TREE *tree) {
    NODE *node = reserve(sizeof(NODE));
    PRINTABLE *printable = reserve(sizeof(PRINTABLE));
    printable->node = node;
    printable->next = tree->printable;
    tree->printable = printable;
    return node;
}

static void add_new_node(TREE *tree, int position, char *current) {
    NODE *node = createNode(tree);
    node->edge = reserve(sizeof(EDGE));
    node->edge->start = position;
    node->edge->end = tree->context->current_end_position;
    ITEM *item = reserve(sizeof(ITEM));
    item->key = current;
    item->value = node;
    put(tree->context->active_node->children, item);
}

void defer_addition(TREE *tree, char *current) {
    tree->context->active_edge = *current;
    tree->context->active_length += 1;
    tree->context->remainder +=1;
}

void *at_position(TREE *tree, int position) {
    return tree->string->get(tree->string->buffer, position);
}

static void add_node(TREE *tree, int position) {
    print_context(tree);
    void *current = at_position(tree, position);
    int remainder = tree->context->remainder;
    if (remainder > 0) {
        void *previous = at_position(tree, position - remainder);
        NODE *node_to_split = get(tree->context->active_node->children, previous)->value;
//        void *body = tree->string->body;
//        if (tree->equals(&body[node_to_split->edge->start + remainder], &body[position])) {
//            log_info("Matched %c to %c", (char)body[node_to_split->edge->start + remainder], (char)body[position]);
//        }
        print_node(node_to_split, tree);
    }
    *tree->context->current_end_position += 1;
    if (get(tree->context->active_node->children, current)) {
        defer_addition(tree, current);
    } else {
        add_new_node(tree, position, current);
    }
}

TREE *create_tree(EQUALS_FUNCTION *equals, HASH_FUNCTION *hash) {
    TREE *tree = create_initial_implicit_tree(equals, hash);
    return tree;
}

void add_string(TREE *tree, STRING *string) {
    tree->string = string;
    int i;
    for (i = 0; i < tree->string->buffer_length; i++) {
        add_node(tree, i);
    }
}