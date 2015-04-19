//
// Created by Michael Melia on 18/04/2015.
//
#include "hash.h"
#include "equals.h"
#include "item.h"
#include "report.h"

#ifndef THESIS_BUCKET_H
#define THESIS_BUCKET_H

typedef struct bucket BUCKET;
void put_in(BUCKET *bucket, ITEM *item);
ITEM *get_from(BUCKET *bucket, void *key);
REPORT *report_on_bucket(BUCKET *bucket);
BUCKET *create_new_bucket(EQUALS_FUNCTION *equals);
long number_of_equality_checks(BUCKET *bucket);

#endif //THESIS_BUCKET_H
