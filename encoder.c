#include <stdio.h>
#include <stdlib.h>
#include "debug.h"
#include "def.h"
#include "mem.h"

BITVEC *initialise_bit_vector();

void add_encoding_for(BITVEC *vector, u_long i);

void write_vector(FILE *to, BITVEC *vector) {
    terminate_vector_for_writing(vector);
    write_vector_to_file(vector, to);
    initialise_vector(vector);
}

BITVEC *initialise_bit_vector() {
    BITVEC *vector = reserve(sizeof(BITVEC));
    initialise_vector(vector);
    return vector;
}

void encode(FILE *from, FILE *to)
{
    char *buffer = reserve(sizeof(char) * 80);
    size_t len = 0;
    int can_write = false;
    BITVEC *vector = initialise_bit_vector();
    unsigned long count = 0;
    while (getline(&buffer, &len, from) != -1) {
        unsigned long i = strtoul(buffer, NULL, 10);
        if (i == -1) {
            if (can_write) {
                write_vector(to, vector);
            } else {
                can_write = true;
            }
        } else {
            add_encoding_for(vector, i + 1);
        }
        count++;
    }
    write_vector(to, vector);
    log_info("read %lu lines", count);
}

int main(int argc, char const *argv[])
{
    if (argc < 3) {
        log_err("usage: %s <from> <to>", argv[0]);
        return 1;
    }
    FILE *from = fopen(argv[1], "r");
    FILE *to = fopen(argv[2], "w");
    encode(from, to);
    fclose(to);
    fclose(from);
    return 0;

}