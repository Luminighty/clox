#ifndef clox_vm_h
#define clox_vm_h


#include "chunk.h"
#include <stdint.h>


#define STACK_MAX 256


typedef struct {
	Chunk* chunk;
	uint8_t* ip;
	Value stack[STACK_MAX];
	Value* stack_top;
} VM;


typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR,
} InterpretResult;


void vm_create();
void vm_free();

InterpretResult vm_interpret(const char* source);
void vm_push(Value value);
Value vm_pop();


#endif // clox_vm_h
