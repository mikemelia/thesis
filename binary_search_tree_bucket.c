//
// Created by Michael Melia on 18/04/2015.
//

#include "equals.h"
#include "bucket.h"
#include "allocation.h"
#include "report.h"
#include "hashtable.h"

typedef struct entry {
    ITEM *item;
    struct entry *left;
    struct entry *right;
} BINARY_ENTRY;

struct bucket {
    BINARY_ENTRY *first;
    EQUALS_FUNCTION *equals;
    long comparisons;
};

void handle(BUCKET *bucket, ITEM *item, BINARY_ENTRY **entry);

static BINARY_ENTRY *create_new_entry(ITEM *item) {
    BINARY_ENTRY *entry = reserve(sizeof(BINARY_ENTRY));
    entry->item = item;
    entry->left = NULL;
    entry->right = NULL;
    return entry;
}

static void add_to_tree(BUCKET *bucket, ITEM *item, BINARY_ENTRY *current) {
    bucket->comparisons++;
    int i = bucket->equals(current->item->key, item->key);
    if (i == 0) {
        current->item->value = item->value;
    } else {
        if (i < 0) {
            handle(bucket, item, &current->left);
        } else {
            handle(bucket, item, &current->right);
        }
    }
}

static BINARY_ENTRY *find_entry_in_tree(BUCKET *bucket, BINARY_ENTRY *current, void *key) {
    if (current == NULL) return NULL;
    bucket->comparisons++;
    int i = bucket->equals(current->item->key, key);
    if (i == 0) {
        return current;
    } else {
        if (i < 0) {
            return find_entry_in_tree(bucket, current->left, key);
        } else {
            return find_entry_in_tree(bucket, current->right, key);
        }
    }
}

void handle(BUCKET *bucket, ITEM *item, BINARY_ENTRY **entry) {
    if (*entry == NULL) {
        *entry = create_new_entry(item);
    } else {
        add_to_tree(bucket, item, *entry);
    }
}

BUCKET *create_new_bucket(EQUALS_FUNCTION *equals) {
    BUCKET *bucket = reserve_zeroed(sizeof(BUCKET));
    bucket->equals = equals;
    bucket->comparisons = 0;
    return bucket;
}

void set_as_root(BUCKET *bucket, ITEM *item) {
    bucket->first = create_new_entry(item);
}

void put_in(BUCKET *bucket, ITEM *item) {
    BINARY_ENTRY *current_first = bucket->first;
    if (current_first != NULL) {
        add_to_tree(bucket, item, current_first);
    } else {
        set_as_root(bucket, item);
    }
}

ITEM *get_from(BUCKET *bucket, void *key) {
    BINARY_ENTRY *start = bucket->first;
    if (start == NULL) {
        return NULL;
    }
    BINARY_ENTRY *entry = find_entry_in_tree(bucket, start, key);
    if (entry != NULL) {
        return entry->item;
    }
    return NULL;
}

REPORT *report_on_bucket(BUCKET *bucket) {
    REPORT *report = reserve(sizeof(REPORT));
    return report;
}

long number_of_equality_checks(BUCKET *bucket) {
    return bucket->comparisons;
}

void reset_bucket_comparison(BUCKET *bucket) {
    bucket->comparisons = 0;
}
