#ifndef sethi_table_h
#define sethi_table_h
#define LOAD_FACTOR 0.5
#include "value.h"

typedef struct {
    ObjString* key;
    Value value;
} Entry;

typedef struct {
    int capacity;
    int count;
    Entry* entries;
} Table;

void initTable(Table* table);
void put(Table* table, ObjString* key, Value value);
Value* get(Table* table, ObjString* key);


#endif