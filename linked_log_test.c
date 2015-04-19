//
// Created by Michael Melia on 18/04/2015.
//
#include "hash.h"
#include "debug.h"

void assertEquals(int actual, int expected) {
    if (expected != actual) {
        log_info("Expected %d but got %d", expected, actual);
    }

}
int main(int argc, char const *argv[]) {
    assertEquals(bucket_for(4, 0), 0);
    assertEquals(bucket_for(8, 0), 0);
    assertEquals(bucket_for(4, 1), 1);
    assertEquals(bucket_for(4, 2), 2);
    assertEquals(bucket_for(4, 3), 2);
    assertEquals(bucket_for(4, 4), 3);
    assertEquals(bucket_for(4, 5), 3);
    assertEquals(bucket_for(8, 2), 2);
    assertEquals(bucket_for(8, 3), 3);
    assertEquals(bucket_for(8, 4), 4);
    assertEquals(bucket_for(8, 5), 5);
    assertEquals(bucket_for(8, 6), 4);
    assertEquals(bucket_for(8, 7), 5);
    assertEquals(bucket_for(8, 8), 6);
    assertEquals(bucket_for(8, 9), 6);
    assertEquals(bucket_for(8, 10), 6);
    assertEquals(bucket_for(8, 11), 6);
    assertEquals(bucket_for(8, 12), 6);
    assertEquals(bucket_for(8, 13), 6);
    assertEquals(bucket_for(8, 14), 6);
    assertEquals(bucket_for(8, 15), 6);
    assertEquals(bucket_for(8, 16), 7);
    assertEquals(bucket_for(8, 17), 7);
    assertEquals(bucket_for(8, 999), 7);
    assertEquals(bucket_for(8, 1010101), 7);
    assertEquals(bucket_for(16, 16), 12);
    assertEquals(bucket_for(16, 17), 13);
    assertEquals(bucket_for(16, 18), 12);
    assertEquals(bucket_for(16, 19), 13);
    assertEquals(bucket_for(16, 32), 14);
    assertEquals(bucket_for(16, 1010101), 15);
    assertEquals(bucket_for(128, 1010100), 127);
    assertEquals(bucket_for(128, 1010101), 127);

}
