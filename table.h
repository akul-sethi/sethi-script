#ifndef clox_table_h
#define clox_table_h

#include "common.h"
#include "value.h"

typedef struct {
  ObjString *key;
  Value value;
} Entry;

typedef struct {
  int count;
  int capacity;
  Entry *entries;
} Table;

// Represents a struct in SethiScript
typedef struct {
  Obj obj;
  // Stores key-value pairs
  Table table;
  // The type of struct
  ObjString *type;
} ObjStruct;

void initTable(Table *table);
Value *get(Table *table, ObjString *key);
void set(Table *table, ObjString *key, Value value);
void grow(Table *table);
void freeTable(Table *table);
ObjString *findStringInTable(Table *table, const char *string, int length,
                             uint32_t hash);
ObjStruct *createStruct(ObjString *type);

#define GET_TABLE(struct) (((ObjStruct *)struct.as.obj)->table)

#endif
