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

//Prints an instruction which takes an operand reprsenting a constant in the constant pool.
static int constantInstruction(const char* name, Chunk* chunk, int offset) {
    printf("%s   ", name);
    int index = chunk->code[offset + 1];
    printf("%4d '", index);
    printValue(chunk->constants.values[index]);
    printf("'");
    return offset + 2;
}

//Prints an instruction which takes an operand representing a constant on the vm stack (indexed from bottom).
static int localInstruction(const char* name, Chunk* chunk, int offset) {
    printf("%s   ", name);
    int index = chunk->code[offset + 1];
    printf("Index from bottom of VM: ");
    printf("%4d", index);
    return offset + 2;
}

//Prints instruction which takes two bytes for an operand which represents the jump length
static int jumpInstruction(const char* name, Chunk* chunk, int offset) {
    printf("%s   ", name);
    uint16_t index = (((uint16_t)chunk->code[offset + 1]) << 8) | chunk->code[offset + 2];
    printf("Length of jump: ");
    printf("%u", index);
    return offset + 3;
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
    case OP_SET_LOC: return localInstruction("OP_SET_LOC", chunk, offset);
    case OP_GET_LOC: return localInstruction("OP_GET_LOC", chunk, offset);
    case OP_JUMP: return jumpInstruction("OP_JUMP", chunk, offset);
    case OP_JUMP_BACK: return jumpInstruction("OP_JUMP_BACK", chunk, offset);
    case OP_JUMP_IF_FALSE: return jumpInstruction("OP_JUMP_IF_FALSE", chunk, offset);
    case OP_AND: return simpleInstruction("OP_AND", offset);
    case OP_OR: return simpleInstruction("OP_OR", offset);
    default:   
        printf("Cannot recognize code: %d\n", code);
        return offset + 1;
    }
}

