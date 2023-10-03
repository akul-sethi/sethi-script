#ifndef sethi_vm_h
#define sethi_vm_h

#define STACK_MAX 256

#include "chunk.h"
#include "value.h"
#include "table.h"

typedef struct {
    Chunk* chunk;
    uint8_t* ip;
    Value stack[STACK_MAX];
    Value* stackTop;
    Obj* objects;
    Table table;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

InterpretResult interpret(const char* source);
void initVM();
void freeVM();
void push(Value val);
Value pop();

#endif
