//
// Created by Michael Melia on 18/04/2015.
//
#include <math.h>
#include "hash.h"
#define LOG2(X) ((unsigned) (8*sizeof (long long) - __builtin_clzll((X))))

static int number_of_slots_needed(int number_of_bits) {
    return pow(2, number_of_bits);
}

static int half_of(int number) {
    return number/2;
}

static int slot_for(int slots, long value) {
    return value % slots;
}

long bucket_for(long hash_code, long slots) {
    unsigned int number_of_bits_needed = (hash_code > 1) ? LOG2(hash_code) : 1;
    int number_of_slots_available = slots - 2;
    int i = 0;
    int start = 0;
    int number_of_slots_taken = 2;
    for (i = 1; i < number_of_bits_needed && (number_of_slots_available > 0); i++) {
        start += number_of_slots_taken;
        int needed = number_of_slots_needed(i);
        int half = half_of(number_of_slots_available);
        int available = (half > 0) ? half : 1;
        number_of_slots_taken = (available > needed) ? needed : available;
        number_of_slots_available -= number_of_slots_taken;
    }
    return start + slot_for(number_of_slots_taken, hash_code - start);
}
