typedef struct hashtable {} HashTable;

HashTable *create_hashtable();
int add(void *object, HashTable *table);