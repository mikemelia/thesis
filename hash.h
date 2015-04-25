//
// Created by Michael Melia on 18/04/2015.
//
#include "string.h"
#ifndef THESIS_HASH_H
#define THESIS_HASH_H

typedef long (HASH_FUNCTION)(void *);

static const int number_of_samples = 1024;

long bucket_for(long hash_code, long slots);

void prepare_with(STRING *string, HASH_FUNCTION *hash_function);

#endif //THESIS_HASH_H
