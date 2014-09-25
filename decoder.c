#include <stdio.h>
#include "debug.h"
#include "def.h"
#include "mem.h"

void add_encoding_for(BITVEC *vector, unsigned long i);
int get_next_decoded_integer_from(BITVEC *vector, unsigned long *i);

BITVEC *initialise_bit_vector();

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

void decode(FILE *from, FILE *to)
{
    BITVEC *vector = initialise_bit_vector();
    unsigned long count = 0;
    unsigned long number;
    while (read_vector_from_file(vector, from) != -1) {
        fprintf(to, "-1\n");
        while (get_next_decoded_integer_from(vector, &number) != -1) {
            fprintf(to, "%lu\n", (number - 1));
            count++;
        }
    }
    log_info("wrote %lu lines", count);
}

int main(int argc, char const *argv[])
{
    if (argc < 3) {
        log_err("usage: %s <from> <to>", argv[0]);
        return 1;
    }
    FILE *from = fopen(argv[1], "r");
    FILE *to = fopen(argv[2], "w");
    decode(from, to);
    fclose(to);
    fclose(from);
    return 0;

}