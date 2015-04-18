//
// Created by Michael Melia on 18/04/2015.
//
#include "hash.h"

int bucket_for(int slots, unsigned long value) {
    return value % slots;
}


