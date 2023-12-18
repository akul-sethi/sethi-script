#ifndef sethi_chunk_h
#define sethi_chunk_h

#include "common.h"
#include "memory.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,
    OP_RETURN,
    OP_NEGATE,
    OP_MUL,
    OP_DIVIDE,
    OP_ADD,
    OP_SUBTRACT,
    OP_TRUE,
    OP_FALSE,
    OP_NIL,
    OP_EQUALITY,
    OP_LESS,
    OP_GREATER,
    OP_GREATER_EQUAL,
    OP_LESS_EQUAL,
    OP_FALSIFY,
    OP_PRINT,
    OP_POP,
    OP_DEFINE_GLOB,
    OP_SET_GLOB,
    OP_GET_GLOB,
    OP_SET_LOC,
    OP_GET_LOC,
    OP_JUMP_IF_FALSE,
    OP_JUMP,
    OP_JUMP_BACK,
    OP_AND,
    OP_OR,
    OP_CALL,
    OP_DOT,
    OP_TABLE,
    OP_NAMESPACE
} OpCode;

typedef struct {
    int32_t count;
    int32_t capacity;
    uint8_t* code;
    int* lines;
    ValueArray constants;
} Chunk;



void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
void freeChunk(Chunk*);
int addConstant(Chunk* chunk, Value val);
// void pickle(Chunk* chunk, const char* path);
// Chunk* unpickle(Chunk* chunk, const char* path);

#endif
