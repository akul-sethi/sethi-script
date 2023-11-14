#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"
#include "value.h"
#include "table.h"
#include "vm.h"


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

void freeObject(Obj* obj) {
    ObjType type = obj->type;
    switch (type)
    {
    case OBJ_STRING: {
        ObjString* ptr = (ObjString*) obj;
        free((void*)ptr->string);
        free((void*)ptr);
        break;
    } 
    case OBJ_FUNCTION: {
        ObjFunc* ptr = (ObjFunc*) obj;
        free((void*)ptr);
        break;
    } 
    case OBJ_STRUCT: {
         ObjStruct* ptr = (ObjStruct*) obj;
         freeTable(&ptr->table);
         free((void*)ptr);
    }
    default:
        break;
    }
}

void freeValueArray(ValueArray* arr) {
    FREE_ARRAY(Value, arr->values, arr->count);
    initValueArray(arr);
}

void printValue(Value val) {
    switch(val.type) {
        case VALUE_BOOL: {
            printf("%s", val.as.boolean ? "true" : "false"); 
            break;
        }
        case VALUE_NIL: {
            printf("nil"); 
            break;
        }
        case VALUE_NUM: {
            printf("%d", val.as.number); 
            break;
        }
        case VALUE_OBJ: 
            switch (val.as.obj->type)
            {
            case OBJ_STRING: printf("%s", ((ObjString*) val.as.obj)->string); break;
            case OBJ_STRUCT: printf("num vals: %d", ((ObjStruct*) val.as.obj)->table.count); break;
            case OBJ_FUNCTION: printf("sc: %d, np: %d", ((ObjFunc*) val.as.obj)->startCount, ((ObjFunc*) val.as.obj)->numParams); break;
            default:
                break;
            }
        default: break;
    }
        
}

bool isObjectOfType(Value val, ObjType type) {
    return IS_OBJ(val) && val.as.obj->type == type;
}

 uint32_t hash(const char* string, int length) {
    uint32_t hash = 2166136261u;
    
    for(int i = 0; i < length; i++) {
        hash ^= (uint32_t)string[i];
        hash *= 16777619;
    }

    return hash;
}

//Checks if string exists in intern table, otherwise creates string in heap, creates objstring, and adds it to table
ObjString* copyString(const char* string, int length) {
    uint32_t hashVal = hash(string, length);
    ObjString* intern = findStringInTable(&vm.strings, string, length, hashVal);
    if(intern != NULL) {return intern;} 
        
    char* heapPtr = (char*) malloc(sizeof(char) * (length + 1));
    if(heapPtr == NULL) {
        exit(1);
    }
    memcpy(heapPtr, string, length);
    heapPtr[length] = '\0';
    ObjString* heapObj = (ObjString*) malloc(sizeof(ObjString));
      if(heapObj == NULL) {
        exit(1);
    }
    ((Obj*)heapObj)->type = OBJ_STRING;
    ((Obj*)heapObj)->next = vm.objects;
    vm.objects = &heapObj->obj;
    heapObj->length = length;
    heapObj->string = heapPtr;
    heapObj->hash = hashVal;
    set(&vm.strings, heapObj, MAKE_NIL());

    return heapObj;
 }

//Creates a ObjFunc on the heap
 ObjFunc* createFunc(int startCount, int numParams) {
    ObjFunc* output = (ObjFunc*) malloc(sizeof(ObjFunc));

    ((Obj*)output)->type = OBJ_FUNCTION;
    ((Obj*)output)->next = vm.objects;
    vm.objects = &output->obj;
    output->startCount = startCount;
    output->numParams = numParams;
    
    return output;
 }



