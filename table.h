#ifndef clox_table_h
#define clox_table_h

#include "common.h"
#include "value.h"


typedef struct {
  ObjString* key;
  Value value;
} Entry;

typedef struct {
  int count;
  int capacity;
  Entry* entries;
} Table;

void initTable(Table* table);
Value* get(Table* table, ObjString* key);
void set(Table* table, ObjString* key, Value value);
void grow(Table* table);
void freeTable(Table* table);
ObjString* findStringInTable(Table* table, const char* string, int length, uint32_t hash);

#endif