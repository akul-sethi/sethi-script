#ifndef sethi_value_h
#define sethi_value_h

#include "common.h"

typedef enum {
    VALUE_NIL,
    VALUE_NUM,
    VALUE_OBJ,
    VALUE_BOOL
} ValueType;

typedef enum {
    OBJ_STRING
} ObjType;

typedef struct Obj Obj;

 struct Obj {
   ObjType type;
   Obj* next;
};

typedef struct {
    Obj obj;
    int length;
    const char* string;
    u_int32_t hash;
} ObjString;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        int number;
        Obj* obj;
    } as;
} Value;

typedef struct {
    int32_t count;
    int32_t capacity;
    Value* values;
} ValueArray;

void initValueArray(ValueArray* arr);
void writeValueArray(ValueArray* arr, Value val);
void freeValueArray(ValueArray* arr);
void printValue(Value val);
bool isObjectOfType(Value val, ObjType type);
ObjString* copyString(const char* string, int length);


#define IS_NIL(value) (value.type==VALUE_NIL)
#define IS_BOOL(value) (value.type==VALUE_BOOL)
#define IS_OBJ(value) (value.type==VALUE_OBJ)
#define IS_NUM(value) (value.type==VALUE_NUM)
#define IS_STRING(value) isObjectOfType(value, OBJ_STRING)

#define MAKE_BOOL(bool) ((Value){.type = VALUE_BOOL, .as.boolean = bool})
#define MAKE_NUM(value) ((Value){.type = VALUE_NUM, .as.number = value})
#define MAKE_NIL() ((Value){.type = VALUE_NIL})
#define MAKE_OBJ(ptr) ((Value) {.type = VALUE_OBJ, .as.obj = ptr})





#endif