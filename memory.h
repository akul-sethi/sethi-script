#ifndef sethi_memory_h
#define sethi_memory_h

#include "common.h"

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*)reallocate(pointer, sizeof(type) * (oldCount), \
        sizeof(type) * (newCount))
#define GROW_CAPACITY(capacity) ((capacity < 8) ? 8 : (capacity * 2))
#define FREE_ARRAY(type, pointer, oldCount) reallocate(pointer, sizeof(type) * oldCount, 0)

void* reallocate(void* pointer, size_t oldSize, size_t newSize);

#endif

