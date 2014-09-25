#include <stdlib.h>
#include "debug.h"
#include "hashtable.h"
#include "mem.h"

int main(int argc, char *argv[]) {
    log_info("Hello World");
    int i = 0;
    log_info("Hello %d", i);
    HashTable *table = reserve(sizeof(HashTable));
    log_info("Found %d", add("This", table));

}