#include "table.h"
#include "memory.h"
#include "string.h"
#include <stdlib.h>

#define LOAD_FACTOR 0.75

void initTable(Table* table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

 void grow(Table* table) {
    int oldCapacity = table->capacity;
     if(table->capacity == 0) {
        table->capacity = 8;
    } else {
        table->capacity = 2 * table->capacity;
    }
    Entry* oldEntries = table->entries;
    table->entries = (Entry*) malloc(table->capacity * sizeof(Entry*));

    for(int i = 0; i < table->capacity; i++) {
        table->entries[i].key = NULL;
        table->entries[i].value = MAKE_NIL();
    }

    for(int i = 0; i < oldCapacity; i++) {
        if(oldEntries[i].key != NULL) {
            set(table, oldEntries[i].key, oldEntries[i].value);
        }
    } 

    free(oldEntries);
    }

void set(Table* table, ObjString* key, Value value) {
    if(table->count / table->capacity > LOAD_FACTOR || table->capacity == 0) {
        grow(table);
    }
    int index = key->hash % table->capacity;
    for(;;) {
        if(table->entries[index].key == NULL || table->entries[index].key == key) {
            table->entries[index].key = key;
            table->entries[index].value = value;
            break;
        }
        index = (index + 1) % table->capacity;
    }

}

Value* get(Table* table, ObjString* key) {
    if(table->entries == NULL) return NULL;
    int index = key->hash % table->capacity;
    for(;;) {
        if(table->entries[index].key == NULL) {return NULL;}
        if(table->entries[index].key == key) {
            return &table->entries[index].value;
        }
        index = (index + 1) % table->capacity;
    }
}

void freeTable(Table* table) {
    table->count = 0;
    table->capacity = 0;
    free(table->entries);
}

ObjString* findStringInTable(Table* table, const char* string, int length, uint32_t hash) {
    if(table->entries == NULL) return NULL;
     int index = hash % table->capacity;
    for(;;) {
        if(table->entries[index].key == NULL) {return NULL;}
        // printf()
        if(strcmp(table->entries[index].key->string,  string) == 0) {
            return table->entries[index].key;
        }
        index = (index + 1) % table->capacity;
    }
    
}



