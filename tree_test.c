#include <stdlib.h>
#include "hashtable.h"
#include "allocation.h"
#include "filereader.h"
#include "tree.h"
#include "debug.h"

long hash_longs(void *value) {
    int *translation = (int *) value;
    return abs(*translation);
}

static int long_equals(void *this, void *that) {
    int *first = (int *) this;
    int *second = (int *) that;
    return (*first > *second) - (*first < *second);
}

static ITEM *create_long_item(long key, long value) {
    ITEM *item = reserve(sizeof(ITEM));
    item->key = reserve(sizeof(int));
    item->value = reserve(sizeof(long));
    *(long *)item->key = key;
    *(long *)item->value = value;
    return item;
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

void test(char *file_name) {
    long number = number_of_lines(file_name) + 1;
    long *buffer = reserve(number * sizeof(long));
    read_into(file_name, buffer);

    STRING *string = reserve(sizeof(STRING));
    string->buffer = buffer;
    string->buffer_length = number;
    string->equals = &equals_long;
    string->get = &get_long;
    string->to_string = &long_to_string;
    prepare_with(string, &hash_longs);

    TREE *tree = create_tree(&long_equals, &hash_longs);
    add_string(tree, string);
    log_info("%ld nodes with %ld puts, %ld gets and %ld comparisons", number_of_nodes(tree), total_number_of_puts(tree), total_number_of_gets(tree), total_number_of_comparisons(tree));
}


int main(int argc, char const *argv[]) {
    test("/Volumes/Flash/thesis/data/postings.txt");
//    test("/Users/michael/Dropbox/University/dev/thesis/geometric1M0.125.txt");
}
