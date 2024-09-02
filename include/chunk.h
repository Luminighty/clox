#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
	OP_RETURN,
	OP_CONSTANT,
	OP_NIL,
	OP_TRUE,
	OP_FALSE,
	OP_NEGATE,
	OP_NOT,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_EQUAL,
	OP_GREATER,
	OP_LESS,
} OpCode;


typedef struct {
	int count;
	int capacity;
	uint8_t* code;
	ValueArray constants;
	int* lines;
} Chunk;


Chunk chunk_create();
void chunk_write(Chunk* chunk, uint8_t byte, int line);
void chunk_free(Chunk* chunk);

int chunk_write_constant(Chunk* chunk, Value value);


#endif // clox_chunk_h
