#ifndef sethi_vm_h
#define sethi_vm_h

#define STACK_MAX 256

#include "chunk.h"
#include "value.h"
#include "table.h"

typedef struct {
    //All Op Codes.
    Chunk* chunk;
    //Points to the current OpCode that has just been read.
    uint8_t* ip;
    //Array of Values representing the stack.
    Value stack[STACK_MAX];
    //Points to the value at the top of the stack
    Value* stackTop;
    //All objects that have been created on the heap.
    Obj* objects;
    //All interned strings.
    Table strings;
    //All global vars.
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
