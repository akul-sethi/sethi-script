#ifndef sethi_debug_h
#define sethi_debug_h

#include "chunk.h"
#include "table.h"

void dissasembleChunk(Chunk* chunk, const char* name);
int dissasembleInstruction(Chunk* chunk, int offset);

#endif
