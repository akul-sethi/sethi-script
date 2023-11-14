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
    vm.frameBottom = 0;
    vm.stackTop = vm.stack;
}

void initVM() {
    resetStack();
    initTable(&vm.table);
    initTable(&vm.strings);
    vm.objects = NULL;
}


//Removes value from top of stack and returns it
Value pop() {
    vm.stackTop--;
    return *vm.stackTop;
}

void push(Value val) {
    *vm.stackTop = val;
    vm.stackTop++;
}

//Returns the Value that is "distance" values away from the top of the stack
Value peek(int distance) {
    return *(vm.stackTop - distance);
}

//Returns a INTERPRET_RUNETIME_ERROR and print the given message, indicating the current line the program is at.
//Adds new line
InterpretResult runtimeError(const char* message) {
    int line = vm.chunk->lines[vm.ip - vm.chunk->code];
    printf("Error at line %d, %s\n", line, message);
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
    //Increments the ip twice and returns the uint16_t value of the next two bytes.
    #define READ_JUMP() ((((uint16_t)*vm.ip++) << 8) + *vm.ip++)
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

    #ifdef DEBUG_TRACE_EXECUTION
        printf("== VM State == \n");
    #endif
    for(;;) {
        #ifdef DEBUG_TRACE_EXECUTION
            dissasembleInstruction(vm.chunk,(int)(vm.ip - vm.chunk->code));
            printf("   stack before (bottom first): ");
            for(Value* slot = vm.stack; slot < vm.stackTop; slot++) {
                printf("[");
                printValue(*slot);
                printf("]");
            }
            printf("\n");
        #endif
        uint8_t instruction = READ_BYTE();
        switch (instruction)
        {
        case OP_RETURN: {
            if(vm.frameBottom == 0) {
                return INTERPRET_OK;
            }
            Value returnVal = pop();
            vm.ip = vm.chunk->code + vm.returnCount;
            vm.stackTop = vm.stack + vm.frameBottom;

            vm.frameBottom = pop().as.number;
            vm.returnCount = pop().as.number;
            push(returnVal);
            break;
        }
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
            printValue(pop());
            printf("\n");
            break;
        case OP_POP: pop(); break;
        case OP_DEFINE_GLOB: {
            ObjString* s = (ObjString*)READ_CONSTANT().as.obj;
            set(&vm.table, s, peek(1));
            pop();
            break;
        } 
        case OP_SET_GLOB: {
            ObjString* s = (ObjString*)READ_CONSTANT().as.obj;
            if(get(&vm.table, s) == NULL) {
                return runtimeError("Global variable is not defined");
            }
            set(&vm.table, s, peek(1));
        
            break;
        }
        case OP_GET_GLOB: {
            ObjString* s =(ObjString*)READ_CONSTANT().as.obj;
            Value* val = get(&vm.table, s);
            if(val == NULL) {
                return runtimeError("Global variable has not been defined");
            } else {
                push(*val);
            }
             break;
        }
        case OP_SET_LOC: {
            uint8_t index = READ_BYTE();
            vm.stack[vm.frameBottom + index] = peek(1);
            break;
        }
        case OP_GET_LOC: {
            uint8_t index = READ_BYTE();
            push(vm.stack[vm.frameBottom + index]);
            break;
        }
        case OP_JUMP_IF_FALSE: {
            uint16_t jumpLength = READ_JUMP();
            if(IS_FALSE(peek(1))) { 
                vm.ip += jumpLength;
            } 
            break;
        }
        case OP_JUMP: {
            uint16_t jumpLength = READ_JUMP();
            vm.ip += jumpLength;
            break;
        }
        case OP_AND: {
            Value temp = MAKE_BOOL(IS_TRUE(peek(1)) && IS_TRUE(peek(2)));
            pop();
            pop();
            push(temp);
            break;
        }
        case OP_OR: {
            Value temp = MAKE_BOOL(IS_TRUE(peek(1)) || IS_TRUE(peek(2)));
            pop();
            pop();
            push(temp);
            break;
        }
        case OP_JUMP_BACK: {
            uint16_t jumpLength = READ_JUMP();
            vm.ip -= jumpLength;
            break;
        } 
        case OP_CALL: {
            Value last = pop();
            if(last.type != VALUE_OBJ || last.as.obj->type != OBJ_FUNCTION) {
                runtimeError("Value is not callable");
            }
            ObjFunc* func = (ObjFunc*)last.as.obj;
            //Patch placeholders
            //Frame bottom on top of return address
            uint8_t numActualParams = READ_BYTE();
            if(func->numParams != numActualParams) {
                for(int i = 0; i < numActualParams + 2; i++) {
                    pop();
                }
                return runtimeError("Invalid number of parameters");
            }
            int currentCount = vm.ip - vm.chunk->code;
            *(vm.stackTop - numActualParams -1) = (Value) {.type = VALUE_NUM, .as.number = vm.frameBottom};
            *(vm.stackTop - numActualParams - 2) = (Value) {.type = VALUE_NUM, .as.number = vm.returnCount};

            vm.frameBottom = (uint8_t)(vm.stackTop - vm.stack) - (uint8_t)(numActualParams);
            vm.returnCount = (int)(vm.ip - vm.chunk->code);

            vm.ip = vm.chunk->code + func->startCount;
            break;
        }

        default:
            return INTERPRET_RUNTIME_ERROR;
        }
    }

    #undef READ_BYTE
    #undef READ_CONSTANT
    #undef READ_JUMP

}


//Frees all objects from the heap as well as their associated strings.
static void freeObjects() {
    while(vm.objects != NULL) {
        Obj* ptr = vm.objects;
        vm.objects = ptr->next;
        freeObject(ptr);
    }
}

//Frees all objects, string table, and global vars table.
void freeVM() {
    freeObjects();
    freeTable(&vm.strings);
    freeTable(&vm.table);
}

InterpretResult interpret(const char* source) { 
    Chunk chunk;
    initChunk(&chunk);
    Compiler compiler;
    initCompiler(&compiler);
    if(!compile(source, &chunk)) {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    } 

 
    vm.chunk = &chunk;
    vm.ip = chunk.code;

    // dissasembleChunk(&chunk, "Chunk");
    InterpretResult result = run();

    freeChunk(&chunk);
    return INTERPRET_OK;
}

