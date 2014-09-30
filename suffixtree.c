#import "tree.h"
#import <string.h>
#import "allocation.h"

int adjust_char(char *unadjusted) {
    if (*unadjusted >= 'a') return (*unadjusted - 'a');
    return (*unadjusted - 'A');
}

int equals(void *first, void *second) {
    return adjust_char((char *)first) == adjust_char((char *)second);
}

unsigned long hash(void *key) {
    return adjust_char((char *)key);
}

int char_equals(void *buffer, unsigned long first, unsigned long second) {
    char *as_char = (char *)buffer;
    return as_char[first] == as_char[second];
}

void *char_get(void *buffer, unsigned long position) {
    char *as_char = (char *)buffer;
    return &as_char[position];
}

char *to_string(void *buffer, unsigned long first, int length) {
    char *as_char = (char *)buffer;
    char *as_string = reserve_zeroed(sizeof(char) * (length + 1));
    strncpy(as_string, &buffer[first], length);
    return as_string;
}

int main(int argc, char const *argv[]) {
    TREE *tree = create_tree(&equals, &hash);
    STRING string;
    string.buffer = "abcabxabcd";
    string.buffer_length = strlen(string.buffer);
    string.equals = &char_equals;
    string.get = &char_get;
    string.to_string = &to_string;
    add_string(tree, &string);
}