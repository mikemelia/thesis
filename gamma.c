#include "def.h"
#include "vec.h"

void add_encoding_for(BITVEC *vector, long i) {
    put_gamma(vector, i);
}

int get_next_decoded_integer_from(BITVEC *vector, long *number) {
    *number = get_gamma(vector);
    return *number;
}

