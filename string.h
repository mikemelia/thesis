//
// Created by Michael Melia on 22/04/2015.
//

#ifndef THESIS_STRING_H
#define THESIS_STRING_H

typedef void *(GET_FUNCTION)(void *buffer, long position);

typedef int (EQUALITY_FUNCTION)(void *buffer, long first, long last);

typedef char *(TO_STRING_FUNCTION)(void *buffer, long first, int length);

typedef struct string {
    void *buffer;
    long buffer_length;
    GET_FUNCTION *get;
    EQUALITY_FUNCTION *equals;
    TO_STRING_FUNCTION *to_string;
} STRING;


#endif //THESIS_STRING_H
