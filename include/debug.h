#ifndef clox_debug_h
#define clox_debug_h

#include "chunk.h"
#include "scanner.h"


void disassemble_chunk(Chunk* chunk, const char* name);
int disassemble_instruction(Chunk* chunk, int offset);
void token_print(Token* token);


#endif // clox_debug_h
