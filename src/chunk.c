#include "chunk.h"
#include "memory.h"
#include "value.h"
#include <stdint.h>


Chunk chunk_create() {
	Chunk chunk;
	chunk.count = 0;
	chunk.capacity = 0;
	chunk.code = NULL;
	chunk.constants = value_array_create();
	chunk.lines = NULL;
	return chunk;
}


void chunk_write(Chunk *chunk, uint8_t byte, int line) {
	if (chunk->capacity < chunk->count + 1) {
		int old_capacity = chunk->capacity;
		chunk->capacity = GROW_CAPACITY(old_capacity);
		chunk->code = GROW_ARRAY(uint8_t, chunk->code, old_capacity, chunk->capacity);
		chunk->lines = GROW_ARRAY(int, chunk->lines, old_capacity, chunk->capacity);
	}

	chunk->code[chunk->count] = byte;
	chunk->lines[chunk->count] = line;
	chunk->count++;
}


void chunk_free(Chunk *chunk) {
	FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
	FREE_ARRAY(int, chunk->lines, chunk->capacity);
	chunk->code = NULL;
	chunk->count = 0;
	chunk->capacity = 0;
	value_array_free(&chunk->constants);
}


int chunk_write_constant(Chunk *chunk, Value value) {
	value_array_write(&chunk->constants, value);
	return chunk->constants.count - 1;
}
