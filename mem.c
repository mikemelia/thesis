#include <stdlib.h>
#include "debug.h"

void *ensure_space_allocated(void *location)
{
    check_mem(location);
    return location;
    error:
    exit(1);
}

void *reserve(size_t amount) {
    return ensure_space_allocated(calloc(1, amount));
}
