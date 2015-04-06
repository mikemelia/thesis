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
    void *active_edge;
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

static char *as_char(void *to_convert) {
    return (char *) to_convert;
}

static void print_context(TREE *tree, int iteration) {
    CONTEXT *context = tree->context;
    debug("CONTEXT for iteration %d:", iteration);
    int i = (context->active_node->edge->end == NULL) ? 0 : *context->active_node->edge->end;
    char *active = context->active_edge == NULL ? ' ' : *as_char(context->active_edge);
    debug("active_node starts at %d and ends at %d with active edge at %c", context->active_node->edge->start, i, active);
    debug("distance_into_active_edge = %d", context->distance_into_active_edge);
    debug("unresolved suffixes = %d", context->unresolved_suffixes);
    debug("current_end_position %d", *context->current_end_position);
}

void print_tree(TREE *tree) {
    PRINTABLE *printable = tree->printable;
    log_info("PRINTING TREE");
    while (printable != NULL) {
        NODE *node = printable->node;
        print_node(node, tree);
        print(node->children);
        printable = printable->next;
    }
}

static int position_in_active_edge(TREE *tree) {
    return tree->context->distance_into_active_edge;
}

static HASH_TABLE *create_children(TREE *tree) {
    return create_hash_table(tree->equals, tree->hash, 1);
}

static CONTEXT *initialise_context(TREE *tree) {
    CONTEXT *context = reserve(sizeof(CONTEXT));
    context->active_node = tree->root;
    context->active_edge = NULL;
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
    tree->root->edge = reserve(sizeof(EDGE));
    tree->root->children = create_children(tree);
    tree->context = initialise_context(tree);
    tree->printable = NULL;
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
    node->suffix_link = NULL;
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
    put(tree->root->children, item);
}

static void decrement_unresolved_suffixes(TREE *tree) {
    tree->context->unresolved_suffixes -= 1;
}

static void increment_unresolved_suffixes(TREE *tree) {
    tree->context->unresolved_suffixes += 1;
}

void defer_addition(TREE *tree, void *current) {
    tree->context->active_edge = current;
    tree->context->distance_into_active_edge += 1;
    increment_unresolved_suffixes(tree);
}

void *at_position(TREE *tree, int position) {
    return tree->string->get(tree->string->buffer, position);
}

static void split_edge(NODE *node_containing_edge, TREE *tree, int latest_position) {
    NODE *node = node_containing_edge;
    debug("Processing from position %d", latest_position);
    debug("Splitting (%d, %d)", node_containing_edge->edge->start, *node_containing_edge->edge->end);
    int split_point = node_containing_edge->edge->start + tree->context->distance_into_active_edge;

    NODE *old_node = create_node(tree);
    old_node->edge->start = split_point;
    old_node->edge->end = reserve(sizeof(int));
    *old_node->edge->end = *node_containing_edge->edge->end;
    old_node->children = node_containing_edge->children;
    debug("Into (%d, %d)", old_node->edge->start, *old_node->edge->end);

    node_containing_edge->children = create_children(tree);

    ITEM *old_item = reserve(sizeof(ITEM));
    old_item->key = at_position(tree, old_node->edge->start);
    old_item->value = old_node;
    put(node_containing_edge->children, old_item);

    NODE *new_node = create_node(tree);
    new_node->edge->start = latest_position;
    new_node->edge->end = tree->context->current_end_position;
    ITEM *new_item = reserve(sizeof(ITEM));
    new_item->key = at_position(tree, new_node->edge->start);
    new_item->value = new_node;
    debug("And (%d, %d)", new_node->edge->start, *new_node->edge->end);

    put(node_containing_edge->children, new_item);

    node_containing_edge->edge->end = reserve(sizeof(int));
    *node_containing_edge->edge->end = split_point - 1;
    debug("leaving (%d, %d)", node_containing_edge->edge->start, *node_containing_edge->edge->end);
}

int edge_length(NODE *node_with_edge) {
    return (*node_with_edge->edge->end - node_with_edge->edge->start + 1);
}

static int next_char_on_edge_matches_latest(TREE *tree, NODE *node_with_edge, int latest_position) {
    STRING *string = tree->string;
    if (position_in_active_edge(tree) >= edge_length(node_with_edge)) {
        return get(node_with_edge->children, string->get(string->buffer, latest_position)) != NULL;
    }
    return string->equals(string->buffer, (node_with_edge->edge->start + tree->context->distance_into_active_edge), latest_position);
}

static void handle_new_entry(TREE *tree, int position) {
    void *current = at_position(tree, position);
    if (get(tree->root->children, current)) {
        defer_addition(tree, current);
    } else {
        add_new_node(tree, position, current);
    }
}

static NODE *create_suffix_link(NODE *previously_split, NODE *node) {
    if (previously_split != NULL) {
        previously_split->suffix_link = node;
    }
    return node;
}

static void move_active_edge_to_next_branch(TREE *tree, int latest_position) {
    tree->context->active_edge = at_position(tree, latest_position - tree->context->unresolved_suffixes + 1);
    tree->context->distance_into_active_edge -= 1;
}

static int *edge_end(NODE *node_to_split) {
    return node_to_split->edge->end;
}

static void *active_edge(TREE *tree) {
    return tree->context->active_edge;
}

static NODE *active_node(TREE *tree) {
    return tree->context->active_node;
}

static HASH_TABLE *children_of_active_node(TREE *tree) {
    CONTEXT *context = tree->context;
    NODE *node = context->active_node;
    return node->children;
}

static int there_are_suffixes_to_add(TREE *tree) {
    return tree->context->unresolved_suffixes;
}

static void *get_value(HASH_TABLE *table, void *key) {
    return get(table, key)->value;
}

static void move_active_node_into_child(TREE *tree, NODE *node_with_active_edge, void *child_key) {
    tree->context->active_node = node_with_active_edge;
    tree->context->distance_into_active_edge = 1;
    tree->context->active_edge = child_key;
}

static void move_active_point_along_one(TREE *tree) {
    tree->context->distance_into_active_edge += 1;
}

static void active_edge_contains_current_char(TREE *tree, int latest_position, NODE *node_with_active_edge) {
    if (position_in_active_edge(tree) >= edge_length(node_with_active_edge)) {
        move_active_node_into_child(tree, node_with_active_edge, at_position(tree, latest_position));
    } else {
        move_active_point_along_one(tree);
    }
    increment_unresolved_suffixes(tree);
}

static void  move_active_node_along_suffix_link(TREE *tree) {
    if (tree->context->active_node->suffix_link == NULL) {
        debug("traversed back to root");
        tree->context->active_node = tree->root;
    } else {
        debug("traversing a suffix link");
        tree->context->active_node = tree->context->active_node->suffix_link;
//        void *edge_to_choose = at_position(tree, *tree->context->current_end_position);
//        NODE *node_with_active_edge = get(tree->context->active_node->children, edge_to_choose)->value;
//        if (position_in_active_edge(tree) > edge_length(node_with_active_edge)) {
//            move_active_node_into_child(tree, node_with_active_edge, edge_to_choose);
//        }
    }
}

static int handle_unresolved_suffixes(TREE *tree, int latest_position) {
    NODE *previously_split = NULL;
    while (there_are_suffixes_to_add(tree)) {
        NODE *node_with_active_point = get_value(children_of_active_node(tree), active_edge(tree));
        if (next_char_on_edge_matches_latest(tree, node_with_active_point, latest_position)) {
            active_edge_contains_current_char(tree, latest_position, node_with_active_point);
            return 0;
        } else {
            split_edge(node_with_active_point, tree, latest_position);
            previously_split = create_suffix_link(previously_split, node_with_active_point);
            if (active_node(tree) != tree->root) {
                move_active_node_along_suffix_link(tree);
            } else {
                move_active_edge_to_next_branch(tree, latest_position);
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
        print_context(tree, i);
    }
    log_info("Processed %ld", tree->string->buffer_length);
    print_hash_usage();
}

POSITIONS *get_positions_matching(TREE *tree, STRING *string) {
    get_value(tree->root->children, string->buffer);
    return NULL;
}