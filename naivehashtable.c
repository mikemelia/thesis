#include <stdlib.h>
#include "hashtable.h"
#include "debug.h"
#include "allocation.h"

void testWithInts();
void testWithChars();

unsigned long charHash(void *value) {
    char *translation = (char *)value;
    int lower = *translation - 'a';
    if (lower < 0) {
        return (*translation - 'A');
    }
    return lower;
}

unsigned long intHash(void *value) {
    int *translation = (int *)value;
    return *translation;
}

int intEquals(void *this, void *that) {
    int *first = (int *)this;
    int *second = (int *)that;
    return *first == *second;
}

int charEquals(void *this, void *that) {
    char *first = (char *)this;
    char *second = (char *)that;
    return *first == *second;
}

ITEM *create_int_item(int key, int value) {
    ITEM *item = reserve(sizeof(ITEM));
    item->key = reserve(sizeof(int));
    item->value = reserve(sizeof(int));
    int *key_a = item->key;
    int *value_a = item->value;
    *key_a = key;
    *value_a = value;
    return item;
}

ITEM *create_char_item(char key, char value) {
    ITEM *item = reserve(sizeof(ITEM));
    item->key = reserve(sizeof(char));
    item->value = reserve(sizeof(char));
    char *key_a = item->key;
    char *value_a = item->value;
    *key_a = key;
    *value_a = value;
    return item;
}

int main(int argc, char const *argv[]) {
    testWithInts();
    testWithChars();
}

void testWithInts() {
    HASH_TABLE *table = create_hash_table(&intEquals, &intHash, 10);
    put(table, create_int_item(1, 2));
    put(table, create_int_item(1, 3));
    put(table, create_int_item(3, 27));
    put(table, create_int_item(13, 200));
    int key;
    for (key = 0; key < 20; key++) {
        ITEM *found = get(table, &key);
        if (found) {
            unsigned long *i = (unsigned long *)found->value;
            log_info("table search for %d found entry with value %lu", key, *i);
        }

    }
}

void testWithChars() {
    HASH_TABLE *table = create_hash_table(&charEquals, &charHash, 10);
    put(table, create_char_item('a', 'b'));
    put(table, create_int_item('c', 'd'));
    put(table, create_int_item('A', 'B'));
    put(table, create_int_item('C', 'D'));
    char key;
    for (key = 'A'; key < 'z'; key++) {
        ITEM *found = get(table, &key);
        if (found) {
            char *i = (char *)found->value;
            log_info("table search for %c found entry with value %c", key, *i);
        }

    }
}