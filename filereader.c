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
    log_info("File size %ld", file_size);
    char *buffer = reserve(file_size + 1);
    fseek(file, 0, SEEK_SET);
    fread(buffer, file_size, 1, file);
    log_info("Read");
    fclose(file);
    buffer[file_size] = 0;
    return buffer;
}
