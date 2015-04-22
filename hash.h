//
// Created by Michael Melia on 18/04/2015.
//

#ifndef THESIS_HASH_H
#define THESIS_HASH_H

typedef long (HASH_FUNCTION)(void *);

long bucket_for(long hash_code, long slots);

#endif //THESIS_HASH_H
