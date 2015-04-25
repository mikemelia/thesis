//
// Created by Michael Melia on 18/04/2015.
//
#include <stdlib.h>
#include <time.h>
#include "hash.h"
#include "string.h"
#include "allocation.h"

static long *samples;

long bucket_for(long hash_code, long slots) {
    int i;
    long factor = number_of_samples/slots;
    long previous_sample = 0;
    for (i = 0; i < slots; i++) {
        long current_count = samples[factor * i];
        if (hash_code >= previous_sample && hash_code <= current_count) {
            int spread = 1;
            int j = i + 1;
            while (j < slots && current_count == samples[factor * j++]) spread++;
            return i + (hash_code % spread);
        }
        previous_sample = current_count;
    }
    return slots - 1;
}
int compare_longs (const void *a, const void *b)
{
    const long *la = (const long *) a;
    const long *lb = (const long *) b;

    return (*la - *lb);
}

void prepare_with(STRING *string, HASH_FUNCTION *hash_function) {
    samples = reserve_zeroed(sizeof(long) * number_of_samples);
    long factor = RAND_MAX/string->buffer_length;
    srand(time(NULL));
    int i;
    for (i = 0; i < number_of_samples; i++) {
        long index = (rand() % string->buffer_length)/factor;
        void *key = string->get(string->buffer, index);
        long hashed_key = hash_function(key);
        samples[i] = hashed_key;
    }
    qsort(samples, number_of_samples, sizeof(long), compare_longs);
}



