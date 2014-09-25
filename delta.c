#include "def.h"
#include "vec.h"

void add_encoding_for(BITVEC *vector, unsigned long i) {
    put_delta(vector, i);
}

int get_next_decoded_integer_from(BITVEC *vector, unsigned long *number) {
    *number = get_delta(vector);
    return *number;
}
