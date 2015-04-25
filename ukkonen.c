#include "allocation.h"
#include "hashtable.h"
#include "debug.h"
#include "tree.h"

typedef struct edge {
    long start;
    long *end;
} EDGE;

typedef struct list_of_tables {
    HASH_TABLE *table;
    struct list_of_tables *next;
} TABLES;

typedef struct node {
    EDGE *edge;
    HASH_TABLE *children;
    struct node *suffix_link;
} NODE;

typedef struct context {
    long *current_end_position;
    NODE *active_node;
    long active_edge;
    long active_length;
    long unresolved_suffixes;
} CONTEXT;

struct tree {
    EQUALS_FUNCTION *equals;
    HASH_FUNCTION *hash;
    NODE *root;
    CONTEXT *context;
    STRING *string;
    TABLES *tables;
    long nodes;
};

static void *at_position(TREE *tree, long position) {
    return tree->string->get(tree->string->buffer, position);
}

static long position_in_active_edge(TREE *tree) {
    return tree->context->active_length;
}

static HASH_TABLE *create_children(TREE *tree) {
    HASH_TABLE *hash_table = create_hash_table(tree->equals, tree->hash, 8);
    TABLES *tables = reserve_zeroed(sizeof(TABLES));
    tables->table = hash_table;
    if (tree->tables != NULL) {
        tables->next = tree->tables;
    }
    tree->tables = tables;
    return hash_table;
}

static CONTEXT *initialise_context(TREE *tree) {
    CONTEXT *context = reserve(sizeof(CONTEXT));
    context->active_node = tree->root;
    context->active_edge = 0;
    context->active_length = 0;
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
    tree->nodes = 0;
    tree->root->edge = reserve(sizeof(EDGE));
    tree->root->children = create_children(tree);
    tree->root->suffix_link = NULL;
    tree->context = initialise_context(tree);
    return tree;
}

static NODE *create_node(TREE *tree) {
    NODE *node = reserve(sizeof(NODE));
    node->edge = reserve(sizeof(EDGE));
    node->children = create_children(tree);
    node->suffix_link = NULL;
    tree->nodes++;
    return node;
}

static void decrement_unresolved_suffixes(TREE *tree) {
    tree->context->unresolved_suffixes -= 1;
}

static void increment_unresolved_suffixes(TREE *tree) {
    tree->context->unresolved_suffixes += 1;
}

static NODE *add_child_to(TREE *tree, NODE *parent, long latest_position) {
    NODE *child = create_node(tree);
    child->edge->start = latest_position;
    child->edge->end = tree->context->current_end_position;
    ITEM *item = reserve(sizeof(ITEM));
    item->key = at_position(tree, child->edge->start);
    item->value = child;
    if (parent == tree->root) {
        debug("Adding (%ld, %ld) to root", child->edge->start, *child->edge->end);
    } else {
        debug("Adding (%ld, %ld) to (%ld, %ld)", child->edge->start, *child->edge->end, parent->edge->start, *parent->edge->end);
    }
    put(parent->children, item);
    return child;
}

static NODE *active_node(TREE *tree) {
    return tree->context->active_node;
}

static NODE *split_edge(NODE *node_containing_edge, TREE *tree, int latest_position) {
    debug("Processing from position %d", latest_position);
    debug("Splitting (%ld, %ld)", node_containing_edge->edge->start, *node_containing_edge->edge->end);
    long split_point = node_containing_edge->edge->start + tree->context->active_length;

    NODE *split_node = create_node(tree);
    split_node->edge->start = node_containing_edge->edge->start;
    split_node->edge->end = reserve(sizeof(int));
    *split_node->edge->end = split_point - 1;
    ITEM *split_item = reserve(sizeof(ITEM));
    split_item->key = at_position(tree, split_node->edge->start);
    split_item->value = split_node;
    put(active_node(tree)->children, split_item);

    ITEM *old_item = reserve(sizeof(ITEM));
    node_containing_edge->edge->start = split_point;
    old_item->key = at_position(tree, node_containing_edge->edge->start);
    old_item->value = node_containing_edge;
    put(split_node->children, old_item);

    NODE *new_node = create_node(tree);
    new_node->edge->start = latest_position;
    new_node->edge->end = tree->context->current_end_position;
    ITEM *new_item = reserve(sizeof(ITEM));
    new_item->key = at_position(tree, latest_position);
    new_item->value = new_node;
    put(split_node->children, new_item);

    debug("Into (%ld, %ld)", split_node->edge->start, *split_node->edge->end);
    debug("with child (%ld, %ld)", node_containing_edge->edge->start, *node_containing_edge->edge->end);
    debug("And child (%ld, %ld)", new_node->edge->start, *new_node->edge->end);
    return split_node;
}

long edge_length(NODE *node_with_edge) {
    return (*node_with_edge->edge->end - node_with_edge->edge->start + 1);
}

static NODE *create_suffix_link(NODE *previously_split, NODE *node) {
    if (previously_split != NULL) {
        previously_split->suffix_link = node;
        debug("CREATING a suffix link from node(%ld, %ld) to node(%ld, %ld)", previously_split->edge->start, *previously_split->edge->end, node->edge->start, *node->edge->end);
    }
    return node;
}

static long active_edge(TREE *tree) {
    return tree->context->active_edge;
}

static HASH_TABLE *children_of_active_node(TREE *tree) {
    CONTEXT *context = tree->context;
    NODE *node = context->active_node;
    return node->children;
}

static long there_are_suffixes_to_add(TREE *tree) {
    return tree->context->unresolved_suffixes;
}

static void *get_value(HASH_TABLE *table, void *key) {
    return get(table, key)->value;
}

NODE *get_node_with_active_point(TREE *tree) {
    return (NODE *)get_value(children_of_active_node(tree), at_position(tree, active_edge(tree)));
}

int reset_active_point(TREE *tree) {
    int reset = FALSE;
    NODE *node_with_active_point = get_node_with_active_point(tree);
    long length_of_edge = (edge_length(node_with_active_point));
    if (position_in_active_edge(tree) >= length_of_edge) {
        reset = TRUE;
        if (tree->context->active_node->edge->end == NULL) {
            debug("moving active node out of root and into (%ld, %ld)", node_with_active_point->edge->start, *node_with_active_point->edge->end);
        } else {
            debug("moving active node out of (%ld, %ld) to (%ld, %ld)", tree->context->active_node->edge->start, *tree->context->active_node->edge->end, node_with_active_point->edge->start, *node_with_active_point->edge->end);
        }
        tree->context->active_edge += length_of_edge;
        tree->context->active_node = node_with_active_point;
        tree->context->active_length -= length_of_edge;
    }
    return reset;
}

static NODE *root_of(TREE *tree) {
    return tree->root;
}

void add_next_character(TREE *tree) {
    *tree->context->current_end_position += 1;
}

void move_to_next_unresolved(TREE *tree, int latest_position) {
    decrement_unresolved_suffixes(tree);

    if (active_node(tree) == root_of(tree) && position_in_active_edge(tree) > 0) {
        tree->context->active_length--;
        tree->context->active_edge = latest_position - tree->context->unresolved_suffixes + 1;
    } else {
        tree->context->active_node = (active_node(tree)->suffix_link == NULL) ? root_of(tree) : active_node(tree)->suffix_link;
    }
}

static void add_node(TREE *tree, int latest_position) {
    add_next_character(tree);
    increment_unresolved_suffixes(tree);

    NODE *previously_inserted = NULL;
    while (there_are_suffixes_to_add(tree)) {
        if (position_in_active_edge(tree) == 0) tree->context->active_edge = latest_position;

        if (get(active_node(tree)->children, at_position(tree, active_edge(tree))) == NULL) {
            add_child_to(tree, active_node(tree), latest_position);
            if (active_node(tree) != root_of(tree)) previously_inserted = create_suffix_link(previously_inserted, active_node(tree));
        } else {
            if (reset_active_point(tree)) continue;
            NODE *node_with_active_edge = get_value(active_node(tree)->children, at_position(tree, active_edge(tree)));
            if (tree->string->equals(tree->string->buffer, node_with_active_edge->edge->start + position_in_active_edge(tree), latest_position) == 0) {
                tree->context->active_length++;
                if (active_node(tree) != root_of(tree)) create_suffix_link(previously_inserted, active_node(tree));
                break;
            }
            NODE *split_node = split_edge(node_with_active_edge, tree, latest_position);
            previously_inserted = create_suffix_link(previously_inserted, split_node);
        }
        move_to_next_unresolved(tree, latest_position);

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

int num_children(NODE *node) {
    int number = 0;
    REPORT *report = report_on(node->children);
    if (report->num_entries == 0) {
        return 1;
    }
    int i = 0;
    for (i = 0; i < report->num_entries; i++) {
        number += num_children((NODE *)report->entries[i]->value);
    }
    return number;
}

int num_positions_matching(TREE *tree, char *pattern) {
    int i = 0;
    long length = strlen(pattern);
    ITEM *item = get(tree->root->children, pattern);
    if (item == NULL) {
        return 0;
    }
    NODE *node = item->value;
    int not_found = TRUE;
    while (i  < length && not_found) {
        long length_of_edge = edge_length(node);
        long length_remaining = length - i;
        char *buffer_as_char = (char *)tree->string->buffer;
        char *start = buffer_as_char + node->edge->start;
        if (length_of_edge > length_remaining) {
            if (strncmp(start, pattern + i, length_remaining)) {
                return 0;
            } else {
                not_found = FALSE;
            }
        } else {
            if (strncmp(start, pattern + i, length_of_edge)) {
                return 0;
            } else {
                i += length_of_edge;
                if (i < length) {
                    item = get(node->children, pattern + i);
                    if (item == NULL) {
                        return 0;
                    }
                    node = item->value;
                }
            }
        }
    }
    return num_children(node);
}


long total_number_of_puts(TREE *tree) {
    TABLES *tables = tree->tables;
    long i = 0;
    while (tables != NULL) {
        i += number_of_puts(tables->table);
        tables = tables->next;
    }
    return i;
}

long total_number_of_gets(TREE *tree) {
    TABLES *tables = tree->tables;
    long i = 0;
    while (tables != NULL) {
        i += number_of_gets(tables->table);
        tables = tables->next;
    }
    return i;
}

long total_number_of_comparisons(TREE *tree) {
    TABLES *tables = tree->tables;
    long i = 0;
    while (tables != NULL) {
        i += number_of_comparisons(tables->table);
        tables = tables->next;
    }
    return i;
}

long number_of_nodes(TREE *tree) {
    return tree->nodes;
}