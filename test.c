#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "table.h"
#include "value.h"
#include "vm.h"

int main(int argc, const char* argv[]) {
   initVM();
   Table table;
   initTable(&table);
   ObjString* obj = (ObjString*) malloc(sizeof(ObjString*));
   Value val = (Value){.type = VALUE_BOOL, .as.boolean = true};
   // set(&table, obj, val);
   // printf("%04x", get(&table, obj));
   uint32_t hashVal = hash("hello", 5);
   printf("%04x\n", findStringInTable(&table, "hello", 5, hashVal));
   ObjString* string = copyString("hello", 5);


   printf("%d\n", copyString("hello", 5) == copyString("hello", 5));
  return 0;
}