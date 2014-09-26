#include "hashtable.h"
#include "allocation.h"

typedef struct entry {
    ITEM *item;
    struct entry *next;
} ENTRY;

struct hash_table {
    HASH_FUNCTION *hash;
    EQUALS_FUNCTION *equals;
    ENTRY **buckets;
    int slots;
};

static int slot_for(int slots, unsigned long value) {
    return value % slots;
}

static ENTRY *find_entry_in_list(EQUALS_FUNCTION *equals, ENTRY *start, void *key) {
    ENTRY *current = start;
    while (!equals(current->item->key, key)) {
        if (current->next == NULL) return NULL;
        current = current->next;
    }
    return current;
}

ITEM *get(HASH_TABLE *table, void *key) {
    int bucket = slot_for(table->slots, table->hash(key));
    ENTRY *entry = table->buckets[bucket];
    if (entry == NULL) return NULL;
    ENTRY *new_entry = find_entry_in_list(table->equals, entry, key);
    if (new_entry == NULL) return NULL;
    return new_entry->item;
}

static ITEM *clone(ITEM *item) {
    ITEM *cloned = reserve(sizeof(ITEM));
    cloned->key = item->key;
    cloned->value = item->value;
    return cloned;
}

static ENTRY *create_new_bucket(ITEM *item) {
    ENTRY *entry = reserve(sizeof(ENTRY));
    entry->item = item;
    entry->next = NULL;
    return entry;
}

static void add_entry_to_bucket(HASH_TABLE *table, int bucket_offset, ITEM *item) {
    ENTRY *new_bucket = create_new_bucket(item);
    new_bucket->next = table->buckets[bucket_offset];
    table->buckets[bucket_offset] = new_bucket;
}

int put(HASH_TABLE *table, ITEM *item) {
    int bucket_offset = slot_for(table->slots, table->hash(item->key));
    if (table->buckets[bucket_offset] == NULL) {
        table->buckets[bucket_offset] = create_new_bucket(item);
    } else {
        ENTRY *bucket = table->buckets[bucket_offset];
        ENTRY *matched = find_entry_in_list(table->equals, bucket, item->key);
        if (matched == NULL) {
            add_entry_to_bucket(table, bucket_offset, item);
        }
    }
    return 0;
}

HASH_TABLE *create_hash_table(EQUALS_FUNCTION *equals, HASH_FUNCTION *hash, int number_of_buckets) {
    HASH_TABLE *table = reserve(sizeof(HASH_TABLE));
    table->buckets = reserve_zeroed(sizeof(ENTRY) *number_of_buckets);
    table->slots = number_of_buckets;
    table->equals = equals;
    table->hash = hash;
    return table;
}
