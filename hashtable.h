#include "equals.h"
#include "hash.h"
#include "report.h"
#include "item.h"
#include "bucket.h"

#ifndef __hashtable_h__

#define __hashtable_h__
struct hash_table;

typedef struct hash_table {
    HASH_FUNCTION *hash;
    EQUALS_FUNCTION *equals;
    BUCKET **buckets;
    int slots;
} HASH_TABLE;

HASH_TABLE *create_hash_table(EQUALS_FUNCTION *equals, HASH_FUNCTION *hash, int number_of_buckets);

void put(HASH_TABLE *table, ITEM *item);

ITEM *get(HASH_TABLE *table, void *key);

long number_of_gets();

REPORT *report_on(HASH_TABLE *table);

#endif