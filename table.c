#include "table.h"
#include "memory.h"
#include "string.h"
#include "value.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>

#define LOAD_FACTOR 0.75

// Initializea given table with count: 0, capacity: 0, and entries: NULL
void initTable(Table *table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
}

// Grows table by multiplying capacity by 2, and reinserting all elements. All
// entries are inititialized to (NULL, nil Value)
void grow(Table *table) {
  int oldCapacity = table->capacity;
  table->count = 0;
  if (table->capacity == 0) {
    table->capacity = 8;
  } else {
    table->capacity = 2 * table->capacity;
  }
  Entry *oldEntries = table->entries;
  table->entries = (Entry *)malloc(table->capacity * sizeof(Entry));

  for (int i = 0; i < table->capacity; i++) {
    table->entries[i].key = NULL;
    table->entries[i].value = MAKE_NIL();
  }

  for (int i = 0; i < oldCapacity; i++) {
    if (oldEntries[i].key != NULL) {
      set(table, oldEntries[i].key, oldEntries[i].value);
    }
  }

  free(oldEntries);
}

// Sets the given key to the given Value
void set(Table *table, ObjString *key, Value value) {
  if (table->capacity == 0 ||
      ((double)table->count / (double)table->capacity) > LOAD_FACTOR) {
    grow(table);
  }
  int index = key->hash % table->capacity;
  for (;;) {
    if (table->entries[index].key == NULL || table->entries[index].key == key) {
      table->entries[index].key = key;
      table->entries[index].value = value;
      table->count = table->count + 1;
      break;
    }
    index = (index + 1) % table->capacity;
  }
}

// Gets the value associated with the given key from the given table.
// Uses ObjString* referential equality
Value *get(Table *table, ObjString *key) {
  if (table->entries == NULL)
    return NULL;
  int index = key->hash % table->capacity;
  for (;;) {
    if (table->entries[index].key == NULL) {
      return NULL;
    }
    if (table->entries[index].key == key) {
      return &table->entries[index].value;
    }
    index = (index + 1) % table->capacity;
  }
}

void freeTable(Table *table) {
  table->count = 0;
  table->capacity = 0;
  free(table->entries);
}

/// @brief Returns NULL if the string does not exist in the table, otherwise
/// returns the string
/// @param table Table being searched in
/// @param string String to be compared to
/// @param length Length of string
/// @param hash Hash of string
/// @return Returns NULL or the string as a ObjString*
ObjString *findStringInTable(Table *table, const char *string, int length,
                             uint32_t hash) {
  if (table->entries == NULL)
    return NULL;
  int index = hash % table->capacity;
  for (;;) {

    if (table->entries[index].key == NULL) {
      return NULL;
    }
    if (table->entries[index].key->length == length &&
        memcmp(table->entries[index].key->string, string, length) == 0) {

      return table->entries[index].key;
    }
    index = (index + 1) % table->capacity;
  }
}

// Creates a Table on the heap
ObjStruct *createStruct(ObjString *type) {
  ObjStruct *output = (ObjStruct *)malloc(sizeof(ObjStruct));

  output->type = type;
  output->obj.type = OBJ_STRUCT;
  output->obj.next = vm.objects;
  vm.objects = &output->obj;

  initTable(&output->table);
  return output;
}
