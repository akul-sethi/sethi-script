#ifndef sethi_value_h
#define sethi_value_h

#include "common.h"

typedef enum { VALUE_NIL, VALUE_NUM, VALUE_OBJ, VALUE_BOOL } ValueType;

typedef enum { OBJ_STRING, OBJ_FUNCTION, OBJ_STRUCT } ObjType;

typedef struct Obj Obj;

struct Obj {
  ObjType type;
  Obj *next;
};

typedef struct {
  Obj obj;
  int length;
  char *string;
  u_int32_t hash;
} ObjString;

typedef struct Chunk Chunk;

typedef struct {
  Obj obj;
  // Points to the chunk where the function is defined
  Chunk *chunk;
  uint8_t numParams;
} ObjFunc;

typedef struct {
  ValueType type;
  union {
    bool boolean;
    int number;
    Obj *obj;
  } as;
} Value;

typedef struct {
  int32_t count;
  int32_t capacity;
  Value *values;
} ValueArray;

void freeObject(Obj *obj);
void initValueArray(ValueArray *arr);
void writeValueArray(ValueArray *arr, Value val);
void freeValueArray(ValueArray *arr);
void printValue(Value val);
bool isObjectOfType(Value val, ObjType type);
ObjString *copyString(const char *string, int length);
uint32_t hash(const char *string, int length);
ObjFunc *createFunc(Chunk *chunk, int numParams);
char *typeName(Value val);

#define IS_NIL(value) (value.type == VALUE_NIL)
#define IS_BOOL(value) (value.type == VALUE_BOOL)
#define IS_OBJ(value) (value.type == VALUE_OBJ)
#define IS_NUM(value) (value.type == VALUE_NUM)
#define IS_STRING(value) isObjectOfType(value, OBJ_STRING)
#define IS_FALSE(value) (IS_BOOL(value) && !value.as.boolean)
#define IS_TRUE(value) (IS_BOOL(value) && value.as.boolean)

#define MAKE_BOOL(bool) ((Value){.type = VALUE_BOOL, .as.boolean = bool})
#define MAKE_NUM(value) ((Value){.type = VALUE_NUM, .as.number = value})
#define MAKE_NIL() ((Value){.type = VALUE_NIL})
// Makes a Value of type VALUE_OBJ given the pointer of the Obj
#define MAKE_OBJ(ptr) ((Value){.type = VALUE_OBJ, .as.obj = ptr})

#endif
