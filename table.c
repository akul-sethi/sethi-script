#include "table.h"
#include "memory.h"
#include "string.h"

void initTable(Table* table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

static u_int32_t hash(ObjString* key) {
    u_int32_t hash = 2166136261;
    
    for(int i = 0; i < key->length; i++) {
        hash ^= key->string[i];
        hash *= 16777619;
    }

    return hash;
}

static bool sameString(ObjString* key1, ObjString* key2) {
    return strcmp(key1->string, key2->string) == 0;
}

void put(Table* table, ObjString* key, Value val) {
    if(table->capacity == 0 || table->count / table->capacity > LOAD_FACTOR) {
        int oldCapacity = table->capacity;
        table->capacity = GROW_CAPACITY(table->capacity);
        table->entries = GROW_ARRAY(Entry, table->entries, oldCapacity, table->capacity);
    }

    int hashVal = hash(key);

    for(int i = 0; i < table->capacity; i++) {
        int index = (hashVal + i) % table->capacity;
        if(table->entries[index].key == NULL || sameString(key, table->entries[index].key)) {
            table->entries[index] = (Entry){.key = key, .value = val};
            return;
        }
    }
}

Value* get(Table* table, ObjString* key) {
    int hashVal = hash(key);

    for(int i = 0; i < table->capacity; i++) {
        int index = (hashVal + i) % table->capacity;
        if(sameString(table->entries[index].key, key)) {
            return &table->entries[index].value;
        }
    }

    return NULL;
}
