#include "tree.h"
#include <string.h>
#include "allocation.h"
#include "filereader.h"
#include "debug.h"


int adjust_char(char *unadjusted) {
    return *unadjusted;
}

int equals(void *first, void *second) {
    return adjust_char((char *) first) == adjust_char((char *) second);
}

unsigned long hash(void *key) {
    return adjust_char((char *) key);
}

int char_equals(void *buffer, unsigned long first, unsigned long second) {
    char *as_char = (char *) buffer;
    return as_char[first] == as_char[second];
}

void *char_get(void *buffer, unsigned long position) {
    char *as_char = (char *) buffer;
    return &as_char[position];
}

char *to_string(void *buffer, unsigned long first, int length) {
    char *as_string = reserve_zeroed(sizeof(char) * (length + 1));
    strncpy(as_string, &buffer[first], length);
    return as_string;
}

int num_matching(char *pattern, char *text) {
    TREE *tree = create_tree(&equals, &hash);
    STRING string;
    string.buffer = text;
    string.buffer_length = strlen(string.buffer);
    string.equals = &char_equals;
    string.get = &char_get;
    string.to_string = &to_string;
    add_string(tree, &string);
    int matching = num_positions_matching(tree, pattern);
    free(tree);
    return matching;
}

static void assert_in(int dataset, char *pattern, char *text, int expected) {
    int matching = num_matching(pattern, text);
    if (matching != expected) {
        log_info("Failed to find %d '%s' in dataset %d, instead got %d", expected, pattern, dataset, matching);
    } else {
        log_info("test passed for pattern %s in dataset %d", pattern, dataset);
    }
}

int main(int argc, char const *argv[]) {

//    string.buffer = read_from("/Volumes/Flash/0ws0110.txt");
//    string.buffer = read_from("/Volumes/Flash/pgwht04.txt");
//    string.buffer = read_from("/Users/michael/Dropbox/University/dev/thesis/test3.txt");
//    string.buffer = read_from("/Users/michael/Dropbox/University/dev/thesis/test2.txt");
    assert_in(1, "ab", "abcabxabcd$", 3);
    assert_in(1, "x", "abcabxabcd$", 1);
    assert_in(1, "a", "abcabxabcd$", 3);
    assert_in(2, "*", read_from("/Users/michael/Dropbox/University/dev/thesis/test.txt"), 9);
    assert_in(2, "**", read_from("/Users/michael/Dropbox/University/dev/thesis/test.txt"), 5);
    assert_in(2, "**T", read_from("/Users/michael/Dropbox/University/dev/thesis/test.txt"), 2);
    assert_in(3, "*", read_from("/Users/michael/Dropbox/University/dev/thesis/test3.txt"), 8);
    assert_in(3, "**", read_from("/Users/michael/Dropbox/University/dev/thesis/test3.txt"), 4);
    assert_in(3, "**", read_from("/Users/michael/Dropbox/University/dev/thesis/test3.txt"), 4);
    assert_in(3, "***", read_from("/Users/michael/Dropbox/University/dev/thesis/test3.txt"), 2);
    assert_in(4, "*", read_from("/Users/michael/Dropbox/University/dev/thesis/test2.txt"), 38);
    assert_in(4, "***", read_from("/Users/michael/Dropbox/University/dev/thesis/test2.txt"), 30);
    assert_in(5, "English", read_from("/Volumes/Flash/0ws0110.txt"), 1);
}

