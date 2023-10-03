#include <stdio.h>
#include "table.h"
#include "value.h"

typedef struct {
    int x;
    double z;
} Holder;

typedef struct {
    int y;
} Other;

int main(int argc, const char* argv[]) {
   Table table;
//    initTable(&table);
   Value val1 = {.type = VALUE_BOOL, .as.boolean = false};
   Value val;
   val.type = VALUE_OBJ;
   ObjString* ptr = (ObjString*) val.as.obj;
   ptr->obj.type = OBJ_STRING;
   ptr->string = "hello";
   ptr->length = 5;
//    put(&table, ptr, val1);
//    printValue(*get(&table, ptr));
}