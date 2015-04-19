//
// Created by Michael Melia on 18/04/2015.
//
#include "equals.h"
#include "hash.h"
#include "item.h"
#include "hashtable.h"
#include "allocation.h"

long number_of_gets(HASH_TABLE *table) {
    return table->gets;
};

long number_of_puts(HASH_TABLE *table) {
    return table->puts;
};

long number_of_comparisons(HASH_TABLE *table) {
    int i = 0;
    long comparisons = 0;
    for (i = 0; i < table->slots; i++) {
        if (table->buckets[i] != NULL) {
            comparisons += number_of_equality_checks(table->buckets[i]);
        }
    }
    return comparisons;
}

static BUCKET *get_bucket(HASH_TABLE *table, void *key) {
    int slot = bucket_for(table->slots, table->hash(key));
    if (table->buckets[slot] == NULL) {
        table->buckets[slot] = create_new_bucket(table->equals);
    }
    return table->buckets[slot];
}

static ITEM *clone(ITEM *item) {
    ITEM *cloned = reserve(sizeof(ITEM));
    cloned->key = item->key;
    cloned->value = item->value;
    return cloned;
}

void put(HASH_TABLE *table, ITEM *item) {
    table->puts++;
    BUCKET *bucket = get_bucket(table, item->key);
    put_in(bucket, clone(item));
}

HASH_TABLE *create_hash_table(EQUALS_FUNCTION *equals, HASH_FUNCTION *hash, int number_of_buckets) {
    HASH_TABLE *table = reserve(sizeof(HASH_TABLE));
    table->buckets = reserve_zeroed(sizeof(BUCKET *) * number_of_buckets);
    table->slots = number_of_buckets;
    table->equals = equals;
    table->hash = hash;
    table->puts = 0;
    table->gets = 0;
    return table;
}

ITEM *get(HASH_TABLE *table, void *key) {
    table->gets++;
    BUCKET *bucket = get_bucket(table, key);
    if (bucket != NULL) {
        return get_from(bucket, key);
    }
    return NULL;
}

REPORT *report_on(HASH_TABLE *table) {
    int i = 0;
    int count = 0;
    for (i = 0; i < table->slots; i++) {
        BUCKET *bucket = table->buckets[i];
        if (bucket != NULL) {
            REPORT *bucket_report = report_on_bucket(bucket);
            count += bucket_report->num_entries;
        }
    }
    REPORT *report = reserve(sizeof(REPORT));
    report->num_entries = count;
    report->entries = reserve(sizeof(ITEM *) * count);
    int k = 0;
    for (i = 0; i < table->slots; i++) {
        BUCKET *bucket = table->buckets[i];
        if (bucket != NULL) {
            REPORT *bucket_report = report_on_bucket(bucket);
            int j = 0;
            for (j = 0; j < bucket_report->num_entries; j++) {
                report->entries[k++] = bucket_report->entries[j];
            }
        }
    }
    return report;

}
