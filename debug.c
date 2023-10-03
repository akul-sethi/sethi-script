#include <stdio.h>
#include "debug.h"
#include "value.h"
#include "table.h"

void dissasembleChunk(Chunk* chunk, const char* name) {
    printf("== %s ==\n", name);

    for(int offset = 0; offset < chunk->count;) {
        offset = dissasembleInstruction(chunk, offset);
        printf("\n");
    }
}

static int simpleInstruction(const char* name, int offset) {
    printf("%s", name);

    return offset + 1;
}

static int constantInstruction(const char* name, Chunk* chunk, int offset) {
    printf("%s   ", name);
    int index = chunk->code[offset + 1];
    printf("%4d '", index);
    printValue(chunk->constants.values[index]);
    printf("'");
    return offset + 2;
}

// static int globalInstruction(const char* name, Chunk* chunk, int offset) {
//     printf("%s   ", name);
//     int index = chunk->code[offset + 1];
//     printValue(chunk->constants.values[index]);
//     printf("=");
//     printValue(*get(table, (ObjString*)chunk->constants.values[index].as.obj));
//     return offset + 2;
// }

int dissasembleInstruction(Chunk* chunk, int offset) {
    printf("%04d  ", offset);

    uint8_t code = chunk->code[offset];
    switch (code)
    {
    case OP_RETURN:
        return simpleInstruction("OP_RETURN", offset);
    case OP_CONSTANT:
        return constantInstruction("OP_CONSTANT", chunk, offset);
    case OP_NEGATE:
        return simpleInstruction("OP_NEGATE", offset);
    case OP_ADD: return simpleInstruction("OP_ADD", offset);
    case OP_SUBTRACT: return simpleInstruction("OP_SUBTRACT", offset);
    case OP_MUL: return simpleInstruction("OP_MUL", offset);
    case OP_DIVIDE: return simpleInstruction("OP_DIVIDE", offset);
    case OP_EQUALITY: return simpleInstruction("OP_EQUALITY", offset);
    case OP_LESS: return simpleInstruction("OP_LESS", offset);
    case OP_GREATER: return simpleInstruction("OP_GREATER", offset);
    case OP_LESS_EQUAL: return simpleInstruction("OP_LESS_EQUAL", offset);
    case OP_GREATER_EQUAL: return simpleInstruction("OP_GREATER_EQUAL", offset);
    case OP_FALSE: return simpleInstruction("OP_FALSE", offset);
    case OP_TRUE: return simpleInstruction("OP_TRUE", offset);
    case OP_FALSIFY: return simpleInstruction("OP_FALSIFY", offset);
    case OP_NIL: return simpleInstruction("OP_NIL", offset);
    case OP_PRINT: return simpleInstruction("OP_PRINT", offset);
    case OP_POP: return simpleInstruction("OP_POP", offset);
    case OP_DEFINE_GLOB: return constantInstruction("OP_DEFINE_GLOB", chunk, offset);
    case OP_SET_GLOB: return constantInstruction("OP_SET_GLOB", chunk, offset);
    case OP_GET_GLOB: return constantInstruction("OP_GET_GLOB", chunk, offset);
    default:   
        printf("Cannot recognize code: %d\n", code);
        return offset + 1;
    }
}
