#include "table.h"
#include "memory.h"
#include "string.h"

void initTable(Table* table) {
    table->count = 0;
    table->capacity = 10;
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

// static void grow(Table* table) {
//     int oldCapacity = table->capacity;
//     Entry* oldEntries = table->entries;
//     table->capacity = GROW_CAPACITY(table->capacity);
//     table->entries = (Entry*) realloc(table->entries, table->capacity);
   
//    for(int i = 0; i < oldCapacity; i++) {
//       if(!oldEntries[i].isEmpty) {
//         put(table, oldEntries[i].key, oldEntries[i].value);
//       }
//    } 
// }

void putIntoEntries() {

}
void put(Table* table, ObjString* key, Value val) {
    // if(table->capacity == 0 || table->count / table->capacity > LOAD_FACTOR) {
    //    grow(table);
    // }

    int hashVal = hash(key);

    for(int i = 0; i < table->capacity; i++) {
        int index = (hashVal + i) % table->capacity;
        if(table->entries[index].isEmpty || sameString(key, table->entries[index].key)) {
            table->entries[index] = (Entry){.key = key, .value = val, .isEmpty = false};
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
