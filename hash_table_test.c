#include <stdlib.h>
#include <time.h>
#include "hashtable.h"
#include "debug.h"
#include "allocation.h"
#include "filereader.h"


long hash_longs(void *value) {
    int *translation = (int *) value;
    return labs(*translation);
}

int long_equals(void *this, void *that) {
    int *first = (int *) this;
    int *second = (int *) that;
    return (*first > *second) - (*first < *second);
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
            long *i = (long *) found->value;
            log_info("table search for %d found entry with value %lu", key, *i);
        }
    }
}


long *as_long(void *buffer) {
    return (long *)buffer;
}

void *get_long(void *buffer, long position) {
    return as_long(buffer) + position;
}

int equals_long(void *buffer, long first, long last) {
    long i = as_long(buffer)[first];
    long j = as_long(buffer)[last];
    return (i > j) - (i < j);
}

char *long_to_string(void *buffer, long first, int last) {
    long *long_buffer = as_long(buffer);
    return ("from %ld to %ld");
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
    long number = number_of_lines(file_name);
    long *buffer = reserve(number * sizeof(long));
    read_into(file_name, buffer);

    STRING *string = reserve(sizeof(STRING));
    string->buffer = buffer;
    string->buffer_length = number;
    string->equals = &equals_long;
    string->get = &get_long;
    string->to_string = &long_to_string;

    prepare_with(string, &hash_longs);
    HASH_TABLE *table = create_hash_table(&long_equals, &hash_longs, 128);
    long i = 0;
    for (i = 0; i < string->buffer_length; ++i) {
        long *key = string->get(string->buffer, i);
        put(table, create_long_item(*key, i));
    }

    log_info("%ld puts, %ld gets with %ld comparisons", number_of_puts(table), number_of_gets(table), number_of_comparisons(table));
    reset_gets(table);
    reset_comparisons(table);
    FILE *file = fopen(file_name, "r");
    i = 0;
    char *line = reserve(sizeof(char) * 128);
    srand(time(NULL));
    while ((line = fgets(line, 128, file)) && (i < 1000000)) {
        if (rand() < RAND_MAX/9) {
            long key = atol(line);
            get(table, &key);
            i++;
        }
    }
    fclose(file);
//
    log_info("%ld puts, %ld gets with %ld comparisons", number_of_puts(table), number_of_gets(table), number_of_comparisons(table));

}

int main(int argc, char const *argv[]) {
//    test_comparisons();
//    test_from_file("/Volumes/Flash/thesis/data/postings.txt");
    test_from_file("/Users/michael/Dropbox/University/dev/thesis/geometric10M0.125.txt");
//    test_from_file("/Users/michael/Dropbox/University/dev/thesis/geometric10M0.5.txt");
//    test_from_file("/Users/michael/Dropbox/University/dev/thesis/xaa");
}
