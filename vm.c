#include "common.h"
#include "vm.h"
#include "debug.h"
#include <stdio.h>
#include "compiler.h"
#include "value.h"
#include "string.h"
#include <stdlib.h>


VM vm;

static void resetStack() {
    vm.stackTop = vm.stack;
}

void initVM() {
    resetStack();
    initTable(&vm.table);
    initTable(&vm.strings);
    vm.objects = NULL;
}

void freeVM() {

}

Value pop() {
    vm.stackTop--;
    return *vm.stackTop;
}

void push(Value val) {
    *vm.stackTop = val;
    vm.stackTop++;
}

Value peek(int distance) {
    return *(vm.stackTop - distance);
}

InterpretResult runtimeError(const char* message) {
    int line = vm.chunk->lines[vm.ip - vm.chunk->code];
    printf("Error at line %d, %s", line, message);
    return INTERPRET_RUNTIME_ERROR;
}


bool sameObject(Value a, Value b) {
    Obj* aObj = a.as.obj;
    Obj* bObj = b.as.obj;

    if(aObj->type != bObj->type) {
        return false;
    }

    switch (aObj->type)
    {
    case OBJ_STRING:  {
        ObjString* objStringA = (ObjString*) aObj;
        ObjString* objStringB = (ObjString*) bObj;
        return objStringA == objStringB;
    }
    default: return false; 
    }
}


static InterpretResult run() {
    #define READ_BYTE() (*vm.ip++)
    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
    #define BINARY_OP(op) do { \
        if(peek(1).type != VALUE_NUM || peek(2).type != VALUE_NUM) { \
            return runtimeError("Can not operate on these types"); \
        } \
        Value b = pop(); \
        Value a = pop(); \
        Value final = {.type = VALUE_NUM, .as.number = (a.as.number op b.as.number)}; \
        push(final); \
    } while(false);
    #define COMP_OP(op) do {\
        if(peek(1).type != VALUE_NUM || peek(2).type != VALUE_NUM) {\
            return runtimeError("Can not operate on these types"); \
        }\
        Value b = pop(); \
        Value a = pop(); \
        Value final = {.type = VALUE_BOOL, .as.boolean = (a.as.number op b.as.number)}; \
        push(final); \
        } while(false);

    for(;;) {
        // #ifdef DEBUG_TRACE_EXECUTION
        //     printf("        ");
        //     for(Value* slot = vm.stack; slot < vm.stackTop; slot++) {
        //         printf("[");
        //         printValue(*slot);
        //         printf("]");
        //     }
        //     printf("\n");
        //     dissasembleInstruction(vm.chunk,(int)(vm.ip - vm.chunk->code));
        // #endif
        uint8_t instruction = READ_BYTE();
        switch (instruction)
        {
        case OP_RETURN:
           return INTERPRET_OK;
        case OP_CONSTANT: {
            push(READ_CONSTANT());
            break;
        }
        case OP_NEGATE:
            if(!IS_NUM(peek(1))) {
                return runtimeError("Can only negate number");
            } 
            push(pop());
            break;
        case OP_ADD: {
            Value b = peek(2);
            Value a = peek(1);
            if(a.type == b.type && IS_STRING(a)) {
                ObjString* b = (ObjString*) pop().as.obj;
                ObjString* a = (ObjString*) pop().as.obj;
                char* output = (char*) malloc(a->length + b->length);
                strcpy(output, a->string);
                strcat(output, b->string);
                ObjString* objString = copyString(output, a->length + b->length);
                free(output);
                push(MAKE_OBJ((Obj*) objString));
            } else {
                BINARY_OP(+);
            }
            break;
        }
        case OP_SUBTRACT: BINARY_OP(-); break;
        case OP_MUL: BINARY_OP(*); break;
        case OP_DIVIDE: BINARY_OP(/); break;
        case OP_FALSE: push(MAKE_BOOL(false)); break;
        case OP_TRUE: push(MAKE_BOOL(true)); break;
        case OP_NIL: push(MAKE_NIL()); break;
        case OP_LESS: COMP_OP(<); break;
        case OP_GREATER: COMP_OP(>); break;
        case OP_LESS_EQUAL: COMP_OP(<=); break;
        case OP_GREATER_EQUAL: COMP_OP(>=); break;
        case OP_EQUALITY: {
            Value b = pop();
            Value a = pop();
            if(a.type != b.type) {
                push(MAKE_BOOL(false));
            } else {
                switch(b.type) {
                    case VALUE_BOOL: push(MAKE_BOOL(b.as.boolean == a.as.boolean)); break;
                    case VALUE_NUM: push(MAKE_BOOL(a.as.number == b.as.number)); break;
                    case VALUE_OBJ: push(MAKE_BOOL(sameObject(a, b))); break;
                    default: push(MAKE_BOOL(true));
                }
            }
         break;
        }
        case OP_FALSIFY: 
            if(peek(1).type != VALUE_BOOL) {
                return runtimeError("Can only falsify booleans");
            }
            push(MAKE_BOOL((!pop().as.boolean)));
            break;
        case OP_PRINT:
            printf("\n");
            printValue(pop());
            break;
        case OP_POP: pop(); break;
        case OP_DEFINE_GLOB: {
            printValue(peek(0));
            set(&vm.table, (ObjString*)READ_CONSTANT().as.obj, peek(0));
            pop();
        } 
        case OP_SET_GLOB: {
            printValue(peek(0));
            set(&vm.table, (ObjString*)READ_CONSTANT().as.obj, peek(0));
            pop();
        }
        case OP_GET_GLOB: {
            Value* val = get(&vm.table, (ObjString*)READ_CONSTANT().as.obj);
            if(val == NULL) {
                push(MAKE_NIL());
            } else {
                push(*val);
            }
        }

        default:
            return INTERPRET_RUNTIME_ERROR;
        }
    }

    #undef READ_BYTE
    #undef READ_CONSTANT

}

static void freeObject(Obj* obj) {
    ObjType type = obj->type;
    switch (type)
    {
    case OBJ_STRING: {
        ObjString* ptr = (ObjString*) obj;
        free((void*)ptr->string);
        free((void*)ptr);
        break;
    }
    default:
        break;
    }
}

static void freeObjects() {
    while(vm.objects != NULL) {
        Obj* ptr = vm.objects;
        vm.objects = ptr->next;
        freeObject(ptr);
    }
}

InterpretResult interpret(const char* source) { 
    Chunk chunk;
    initChunk(&chunk);

    if(!compile(source, &chunk)) {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    } 

    initVM();
    vm.chunk = &chunk;
    vm.ip = chunk.code;

    dissasembleChunk(&chunk, "Test");
    InterpretResult result = run();
    freeObjects();
    freeChunk(&chunk);
    freeTable(&vm.strings);
    return result;
}

