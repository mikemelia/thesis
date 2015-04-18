//
// Created by Michael Melia on 18/04/2015.
//

#ifndef THESIS_HASH_H
#define THESIS_HASH_H

typedef unsigned long (HASH_FUNCTION)(void *);

int bucket_for(int slots, unsigned long value);

#endif //THESIS_HASH_H
