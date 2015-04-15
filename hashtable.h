#ifndef __hashtable_h__
#define __hashtable_h__
typedef struct item {
    void *key;
    void *value;
} ITEM;

struct hash_table;

typedef struct report {
    int num_entries;
    ITEM **entries;
} REPORT;

typedef struct hash_table HASH_TABLE;

typedef unsigned long (HASH_FUNCTION)(void *);

typedef int (EQUALS_FUNCTION)(void *, void *);

HASH_TABLE *create_hash_table(EQUALS_FUNCTION *equals, HASH_FUNCTION *hash, int number_of_buckets);

int put(HASH_TABLE *table, ITEM *item);

void print(HASH_TABLE *table);

ITEM *get(HASH_TABLE *table, void *value);

void print_hash_usage();

REPORT *report_on(HASH_TABLE *table);

#endif