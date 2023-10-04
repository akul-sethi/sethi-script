#include "table.h"
#include "memory.h"
#include "string.h"

#define LOAD_FACTOR 0.75

void initTable(Table* table) {
    table->count = 0;
    table->capacity = 10;
    table->entries = calloc(10*sizeof(table));
}

static void grow(Table* table) {
    table->capacity = 2 * table->capacity;
    int oldCapacity = table->capacity;
    Entry* oldEntries = table->entries;
    table->entries = calloc(sizeof(Entry*) * table->capacity);

    for(int i = 0; i < oldCapacity; i++) 
        if(oldEntries[i] != NULL) {
        }
        set(table, )
    } 

    free(oldEntries);
}

void set(Table* table, ObjString* key, Value value) {
    if(table->count / table->capacity > LOAD_FACTOR) {
        grow(table);
    }
}


