#include <stdlib.h>
#include <stdio.h>
#include "chunk.h"


void initChunk(Chunk* chunk) {
    chunk->capacity = 0;
    chunk->count = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}

//Increments chunk top pointer by one and sets the op there to the given byte. Dynamically resizes chunk if it exceeds space.
void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    if(chunk->capacity < chunk->count + 1) {
        uint32_t oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(chunk->capacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
        chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

//Writes a constant to the constant pool and returns its index in the pool
int addConstant(Chunk* chunk, Value val) {
    writeValueArray(&chunk->constants, val);
    return (chunk->constants).count - 1;
}

// void pickle(Chunk* chunk, const char* path) {
//     FILE* ptr = fopen(path, "wb");

//     fprintf(ptr, "%d|%d|%d|%d|", chunk->count, chunk->capacity, chunk->constants.count, chunk->constants.capacity);
//     for(int i = 0; i < chunk->constants.count; i++) {
//         fprintf(ptr, "%d|", chunk->constants.values[i]);
//     }
//     fwrite(chunk->code, 1, chunk->count, ptr);

//     fclose(ptr);
// }

// Chunk* unpickle(Chunk* chunk, const char* path) {
//     initChunk(chunk);

//     FILE* ptr = fopen(path, "rb");

//     fscanf(ptr, "%d|%d|%d|%d|", &chunk->count, &chunk->capacity, &chunk->constants.count, &chunk->constants.capacity);

//     uint8_t* codeAddress = (uint8_t*)malloc(sizeof(chunk->capacity));
//     Value* valuesAddress = (Value*)malloc(sizeof(chunk->constants.capacity));

//      for(int i = 0; i < chunk->constants.count; i++) {
//         fscanf(ptr, "%d|", &valuesAddress[i]);
//     }

//     fread(codeAddress, 1, chunk->count, ptr);

//     fclose(ptr);

//     chunk->code = codeAddress;
//     chunk->constants.values = valuesAddress;
//     return chunk;
// }