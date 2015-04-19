#include <stdlib.h>
#include "hashtable.h"
#include "debug.h"
#include "allocation.h"


unsigned long hash_longs(void *value) {
    int *translation = (int *) value;
    return *translation;
}

int long_equals(void *this, void *that) {
    int *first = (int *) this;
    int *second = (int *) that;
    return *first == *second;
}

ITEM *create_long_item(long key, long value) {
    ITEM *item = reserve(sizeof(ITEM));
    item->key = reserve(sizeof(int));
    item->value = reserve(sizeof(long));
    *(long *)item->key = key;
    *(long *)item->value = value;
    return item;
}

void test() {
    HASH_TABLE *table = create_hash_table(&long_equals, &hash_longs, 5);
    put(table, create_long_item(1, 2));
    put(table, create_long_item(1, 3));
    put(table, create_long_item(3, 27));
    put(table, create_long_item(13, 200));
    int key;
    for (key = 0; key < 20; key++) {
        ITEM *found = get(table, &key);
        if (found) {
            unsigned long *i = (unsigned long *) found->value;
            log_info("table search for %d found entry with value %lu", key, *i);
        }
    }
}

void test_comparisons() {
    HASH_TABLE *table = create_hash_table(&long_equals, &hash_longs, 32);
    put(table, create_long_item(1, 2));
    put(table, create_long_item(1, 3));
    put(table, create_long_item(3, 27));
    put(table, create_long_item(13, 200));
    log_info("%ld puts, %ld gets with %ld comparisons", number_of_puts(table), number_of_gets(table), number_of_comparisons(table));
}

void test_from_file(char *file_name) {
    HASH_TABLE *table = create_hash_table(&long_equals, &hash_longs, 5);
    FILE *file = fopen(file_name, "r");
    char *line;
    size_t length = 128;
    long i = 0;
    while ((line = fgetln(file, &length))) {
        put(table, create_long_item(atol(line), i));
    }
    fclose(file);
    log_info("%ld puts, %ld gets with %ld comparisons", number_of_puts(table), number_of_gets(table), number_of_comparisons(table));
}

int main(int argc, char const *argv[]) {
    test_comparisons();
    test_from_file("/Volumes/Flash/thesis/data/postings.txt");
//    test_from_file("/Users/michael/Dropbox/University/dev/thesis/geometric10M0.5.txt");
}
