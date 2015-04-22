//
// Created by Michael Melia on 18/04/2015.
//
#include <stdlib.h>
#include "hash.h"

long bucket_for(long hash_code, long slots) {
    return abs(hash_code) % slots;
}


