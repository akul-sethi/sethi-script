#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "value.h"


void initValueArray(ValueArray* arr) {
    arr->count = 0;
    arr->capacity = 0;
    arr->values = NULL;
}

void writeValueArray(ValueArray* arr, Value val) {
    if(arr->capacity < arr->count + 1) {
        uint32_t oldCapacity = arr->capacity;
        arr->capacity = GROW_CAPACITY(arr->capacity);
        arr->values = GROW_ARRAY(Value, arr->values, oldCapacity, arr->capacity);
    }

    arr->values[arr->count] = val;
    arr->count++;
}

void freeValueArray(ValueArray* arr) {
    FREE_ARRAY(Value, arr->values, arr->count);
    initValueArray(arr);
}

void printValue(Value val) {
    switch(val.type) {
        case VALUE_BOOL: printf("%s", val.as.boolean ? "true" : "false"); break;
        case VALUE_NIL: printf("nil"); break;
        case VALUE_NUM: printf("%d", val.as.number); break;
        case VALUE_OBJ: 
            if(val.as.obj->type == OBJ_STRING) {
                printf("%s", ((ObjString*) val.as.obj)->string);
            } break;
        default: break;

    }
}

bool isObjectOfType(Value val, ObjType type) {
    return IS_OBJ(val) && val.as.obj->type == type;
}
