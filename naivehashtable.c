#include "hashtable.h"
#include "allocation.h"
#include "debug.h"

typedef struct entry {
    ITEM *item;
    struct entry *next;
} LINKED_ENTRY;

struct hash_table {
    HASH_FUNCTION *hash;
    EQUALS_FUNCTION *equals;
    LINKED_ENTRY **buckets;
    int slots;
};

static long number_of_gets = 0;
static long number_of_comparisons = 0;

static int slot_for(int slots, unsigned long value) {
    return value % slots;
}

static LINKED_ENTRY *find_entry_in_list(EQUALS_FUNCTION *equals, LINKED_ENTRY *start, void *key) {
    LINKED_ENTRY *current = start;
    number_of_comparisons += 1;
    while (!equals(current->item->key, key)) {
        if (current->next == NULL) return NULL;
        current = current->next;
        number_of_comparisons += 1;
    }
    return current;
}
void print_hash_usage() {
    log_info("%ld gets with %ld comparisons", number_of_gets, number_of_comparisons);
}

ITEM *get(HASH_TABLE *table, void *key) {
    number_of_gets += 1;
    int bucket = slot_for(table->slots, table->hash(key));
    LINKED_ENTRY *entry = table->buckets[bucket];
    if (entry == NULL) return NULL;
    LINKED_ENTRY *new_entry = find_entry_in_list(table->equals, entry, key);
    if (new_entry == NULL) return NULL;
    return new_entry->item;
}

static ITEM *clone(ITEM *item) {
    ITEM *cloned = reserve(sizeof(ITEM));
    cloned->key = item->key;
    cloned->value = item->value;
    return cloned;
}

static LINKED_ENTRY *create_new_bucket(ITEM *item) {
    LINKED_ENTRY *entry = reserve(sizeof(LINKED_ENTRY));
    entry->item = item;
    entry->next = NULL;
    return entry;
}

static void add_entry_to_bucket(HASH_TABLE *table, int bucket_offset, ITEM *item) {
    LINKED_ENTRY *new_bucket = create_new_bucket(item);
    new_bucket->next = table->buckets[bucket_offset];
    table->buckets[bucket_offset] = new_bucket;
}

int put(HASH_TABLE *table, ITEM *item) {
    int bucket_offset = slot_for(table->slots, table->hash(item->key));
    if (table->buckets[bucket_offset] == NULL) {
        table->buckets[bucket_offset] = create_new_bucket(item);
    } else {
        LINKED_ENTRY *bucket = table->buckets[bucket_offset];
        LINKED_ENTRY *matched = find_entry_in_list(table->equals, bucket, item->key);
        if (matched == NULL) {
            add_entry_to_bucket(table, bucket_offset, item);
        }
    }
    return 0;
}

void print_entry(ITEM *item) {
    char *key = (char *)item->key;
    log_info("Item has key of %c", *key);
}

void print_bucket(LINKED_ENTRY *bucket, int slot) {
    log_info("Info for bucket id %d", slot);
    LINKED_ENTRY *entry = bucket;
    while (entry != NULL) {
        print_entry(entry->item);
        entry = entry->next;
    }
}

void print(HASH_TABLE *hash_table) {
    int i = 0;
    for (i = 0; i < hash_table->slots; i++) {
        LINKED_ENTRY *bucket = hash_table->buckets[i];
        if (bucket != NULL) print_bucket(bucket, i);
    }
}

HASH_TABLE *create_hash_table(EQUALS_FUNCTION *equals, HASH_FUNCTION *hash, int number_of_buckets) {
    HASH_TABLE *table = reserve(sizeof(HASH_TABLE));
    table->buckets = reserve_zeroed(sizeof(LINKED_ENTRY) * number_of_buckets);
    table->slots = number_of_buckets;
    table->equals = equals;
    table->hash = hash;
    return table;
}

REPORT *report_on(HASH_TABLE *table) {
    int i = 0;
    int count = 0;
    for (i = 0; i < table->slots; i++) {
        LINKED_ENTRY *bucket = table->buckets[i];
        if (bucket != NULL) {
            while (bucket != NULL) {
                count ++;
                bucket = bucket->next;
            }
        }
    }
    REPORT *report = reserve(sizeof(REPORT));
    report->num_entries = count;
    report->entries = reserve(sizeof(ITEM *) * count);
    int j = 0;
    for (i = 0; i < table->slots; i++) {
        LINKED_ENTRY *bucket = table->buckets[i];
        if (bucket != NULL) {
            while (bucket != NULL) {
                report->entries[j++] = bucket->item;
                bucket = bucket->next;
            }
        }
    }
    return report;
}