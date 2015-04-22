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
    struct entry *next;
} LINKED_ENTRY;

struct bucket {
    LINKED_ENTRY *first;
    EQUALS_FUNCTION *equals;
    long comparisons;
};

static LINKED_ENTRY *find_entry_in_list(BUCKET* bucket, LINKED_ENTRY *start, void *key) {
    LINKED_ENTRY *current = start;
    bucket->comparisons += 1;

    while (!bucket->equals(current->item->key, key)) {
        if (current->next == NULL) return NULL;
        current = current->next;
        bucket->comparisons += 1;
    }
    return current;
}

static LINKED_ENTRY *create_new_entry(ITEM *item) {
    LINKED_ENTRY *entry = reserve(sizeof(LINKED_ENTRY));
    entry->item = item;
    entry->next = NULL;
    return entry;
}

static void place_first(BUCKET *bucket, ITEM *item, LINKED_ENTRY *current_first) {
    bucket->first = create_new_entry(item);
    bucket->first->next = current_first;
}

static void add_to_bucket(BUCKET *bucket, ITEM *item, LINKED_ENTRY *current_first) {
    LINKED_ENTRY *exists = find_entry_in_list(bucket, current_first, item->key);
    if (exists == NULL) {
        place_first(bucket, item, current_first);
    } else {
        exists->item = item;
    }
}
BUCKET *create_new_bucket(EQUALS_FUNCTION *equals) {
    BUCKET *bucket = reserve_zeroed(sizeof(BUCKET));
    bucket->equals = equals;
    bucket->comparisons = 0;
    return bucket;
}

void put_in(BUCKET *bucket, ITEM *item) {
    LINKED_ENTRY *current_first = bucket->first;
    if (current_first != NULL) {
        add_to_bucket(bucket, item, current_first);
    } else {
        place_first(bucket, item, current_first);
    }
}

ITEM *get_from(BUCKET *bucket, void *key) {
    LINKED_ENTRY *start = bucket->first;
    if (start == NULL) {
        return NULL;
    }
    LINKED_ENTRY *entry = find_entry_in_list(bucket, start, key);
    if (entry != NULL) {
        return entry->item;
    }
    return NULL;
}

REPORT *report_on_bucket(BUCKET *bucket) {
    REPORT *report = reserve(sizeof(REPORT));
    LINKED_ENTRY *entry = bucket->first;
    report->num_entries = 0;
    while (entry != NULL) {
        report->num_entries ++;
        entry = entry->next;
    }
    report->entries = reserve(sizeof(ITEM *) * report->num_entries);
    entry = bucket->first;
    int i = 0;
    while (entry != NULL) {
        report->entries[i++] = entry->item;
        entry = entry->next;
    }
    return report;
}

long number_of_equality_checks(BUCKET *bucket) {
    return bucket->comparisons;
}

void reset_bucket_comparison(BUCKET *bucket) {
    bucket->comparisons = 0;
}
