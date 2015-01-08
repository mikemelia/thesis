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
    struct node *suffix_link;
} NODE;

typedef struct context {
    int *current_end_position;
    NODE *active_node;
    char active_edge;
    int distance_into_active_edge;
    int unresolved_suffixes;
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


static void print_node(NODE *node, TREE *tree) {
    int start = node->edge->start;
    int end = *node->edge->end;
    char *label = tree->string->to_string(tree->string->buffer, start, end - start + 1);
    log_info("NODE: edge %s from %d to %d", label, start, end);
}

static void print_context(TREE *tree, int iteration) {
    CONTEXT *context = tree->context;
    log_info("CONTEXT for iteration %d:", iteration);
    log_info("distance_into_active_edge = %d", context->distance_into_active_edge);
    log_info("unresolved suffixes = %d", context->unresolved_suffixes);
    log_info("current_end_position %d", *context->current_end_position);
}

void print_tree(TREE *tree) {
    PRINTABLE *printable = tree->printable;
    while (printable) {
        NODE *node = printable->node;
        print_node(node, tree);
        printable = printable->next;
    }
}

static HASH_TABLE *create_children(TREE *tree) {
    return create_hash_table(tree->equals, tree->hash, 26);
}

static CONTEXT *initialise_context(TREE *tree) {
    CONTEXT *context = reserve(sizeof(CONTEXT));
    context->active_node = tree->root;
    context->active_edge = 0;
    context->distance_into_active_edge = 0;
    context->unresolved_suffixes = 0;
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

static NODE *create_node(TREE *tree) {
    NODE *node = reserve(sizeof(NODE));
    node->edge = reserve(sizeof(EDGE));
    PRINTABLE *printable = reserve(sizeof(PRINTABLE));
    printable->node = node;
    printable->next = tree->printable;
    tree->printable = printable;
    node->children = create_children(tree);
    return node;
}

static void add_new_node(TREE *tree, int position, char *current) {
    NODE *node = create_node(tree);
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
    tree->context->distance_into_active_edge += 1;
    tree->context->unresolved_suffixes += 1;
}

void *at_position(TREE *tree, int position) {
    return tree->string->get(tree->string->buffer, position);
}

static void split_nodes(NODE *node_to_split, TREE *tree, int latest_position) {
    NODE *old_node = create_node(tree);
    old_node->edge->start = tree->context->distance_into_active_edge + 1;
    old_node->edge->end = tree->context->current_end_position;

    ITEM *old_item = reserve(sizeof(ITEM));
    old_item->key = at_position(tree, (tree->context->distance_into_active_edge + 1));
    old_item->value = old_node;
    put(node_to_split->children, old_item);

    NODE *new_node = create_node(tree);
    new_node->edge->start = latest_position;
    new_node->edge->end = tree->context->current_end_position;
    ITEM *new_item = reserve(sizeof(ITEM));
    new_item->key = at_position(tree, latest_position);
    new_item->value = new_node;

    put(node_to_split->children, new_item);

    int *split_point = reserve(sizeof(int));
    *split_point = tree->context->distance_into_active_edge;
    node_to_split->edge->end = split_point;
}

static int node_already_contains_current_item(TREE *tree, int latest_position, NODE *node_to_split) {
    STRING *string = tree->string;
    return string->equals(string->buffer, (node_to_split->edge->start + tree->context->distance_into_active_edge), latest_position);
}

static void handle_new_entry(TREE *tree, int position) {
    void *current = at_position(tree, position);
    if (get(tree->context->active_node->children, current)) {
        defer_addition(tree, current);
    } else {
        add_new_node(tree, position, current);
    }
}

static char *as_char(void *to_convert) {
    return (char *) to_convert;
}

static void create_suffix_link(NODE *previously_split, NODE *node_to_split) {
    node_to_split->suffix_link = previously_split;
    previously_split = node_to_split;
}

static void move_active_edge_to_next_branch(TREE *tree, int latest_position) {
    tree->context->active_edge = *as_char(at_position(tree, latest_position - tree->context->unresolved_suffixes + 1));
    tree->context->distance_into_active_edge -= 1;
}

static int position_in_active_edge(TREE *tree) {
    return tree->context->distance_into_active_edge;
}

static int *edge_end(NODE *node_to_split) {
    return node_to_split->edge->end;
}

static char *active_edge(TREE *tree) {
    return &tree->context->active_edge;
}

static HASH_TABLE *children_of_active_node(TREE *tree) {
    return tree->context->active_node->children;
}

static void decrement_unresolved_suffixes(TREE *tree) {
    tree->context->unresolved_suffixes -= 1;
}

static void increment_unresolved_suffixes(TREE *tree) {
    tree->context->unresolved_suffixes += 1;
}

static int there_are_suffixes_to_add(TREE *tree) {
    return tree->context->unresolved_suffixes;
}

static void *get_value(HASH_TABLE *table, char *active_edge_label) {
    return get(table, active_edge_label)->value;
}

static void move_active_node_into_child(TREE *tree, int latest_position, NODE *node_to_split) {
    tree->context->active_node = get_value(node_to_split->children, at_position(tree, latest_position));
    tree->context->distance_into_active_edge = 0;
}

static void move_active_point_along_one(TREE *tree) {
    tree->context->distance_into_active_edge += 1;
}

static void active_edge_contains_current(TREE *tree, int latest_position, NODE *node_with_active_edge) {
    if (*edge_end(node_with_active_edge) < position_in_active_edge(tree)) {
        move_active_node_into_child(tree, latest_position, node_with_active_edge);
    } else {
        move_active_point_along_one(tree);
    }
    increment_unresolved_suffixes(tree);
}

static void move_active_point_along_suffix_link(TREE *tree, NODE *node_with_active_edge) {
    if (node_with_active_edge->suffix_link == NULL) {
        tree->context->active_node = tree->root;
    } else {
        tree->context->active_node = node_with_active_edge->suffix_link;
    }
}

static int handle_unresolved_suffixes(TREE *tree, int latest_position) {
    NODE *previously_split;
    while (there_are_suffixes_to_add(tree)) {
        NODE *node_with_active_edge = get_value(children_of_active_node(tree), active_edge(tree));
        if (node_already_contains_current_item(tree, latest_position, node_with_active_edge)) {
            active_edge_contains_current(tree, latest_position, node_with_active_edge);
            return 0;
        } else {
            split_nodes(node_with_active_edge, tree, latest_position);
            if (node_with_active_edge != tree->root) {
                move_active_point_along_suffix_link(tree, node_with_active_edge);
            } else {
                move_active_edge_to_next_branch(tree, latest_position);
                create_suffix_link(previously_split, node_with_active_edge);
            }
            decrement_unresolved_suffixes(tree);
        }
    }
    return 1;
}



static void add_node(TREE *tree, int position) {
    *tree->context->current_end_position += 1;
    if (handle_unresolved_suffixes(tree, position)) handle_new_entry(tree, position);
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
        print_tree(tree);
        print_context(tree, i);

    }
}