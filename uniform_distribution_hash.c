//
// Created by Michael Melia on 18/04/2015.
//
#include <stdlib.h>
#include "hash.h"
#include "string.h"

long bucket_for(long hash_code, long slots) {
    return labs(hash_code) % slots;
}

void prepare_with(STRING *string, HASH_FUNCTION *hash_function) {
}



