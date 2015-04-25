//
// Created by Michael Melia on 30/03/15.
//
#include <stdio.h>
#include <stdlib.h>
#include "filereader.h"
#include "allocation.h"
#include "debug.h"

char *read_from(char *file_name) {
    FILE *file = fopen(file_name, "r");
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    char *buffer = reserve(file_size + 1);
    fseek(file, 0, SEEK_SET);
    fread(buffer, file_size, 1, file);
    fclose(file);
    buffer[file_size] = 0;
    return buffer;
}

long number_of_lines(char *file_name) {
    FILE *file = fopen(file_name, "r");
    char *line = reserve(128 * sizeof(char));
    long i = 0;
    while ((line = fgets(line, 128, file)) && i < 10000000) {
        ++i;
    }
    fclose(file);
    return i;
}

void read_into(char *file_name, long *array) {
    FILE *file = fopen(file_name, "r");
    char *line = reserve(128 * sizeof(char));
    long i = 0;
    while ((line = fgets(line, 128, file)) && i < 10000000) {
        array[i++] = atol(line);
    }
    array[i] = -2;
    fclose(file);
}
