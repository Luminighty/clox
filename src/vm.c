#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "value.h"
#include <stdint.h>
#include <stdio.h>
#include "vm.h"


VM vm;

static void reset_stack() {
	vm.stack_top = vm.stack;
}


void vm_create() {
	reset_stack();

}


void vm_free() {

}


static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op) \
	do { \
		double b = vm_pop(); \
		double a = vm_pop(); \
		vm_push(a op b); \
	} while (false)


	for (;;) {

		#ifdef DEBUG_TRACE_EXECUTION
		printf("          ");
		for (Value* slot = vm.stack; slot < vm.stack_top; slot++) {
			printf("[ ");
			value_print(*slot);
			printf(" ]");
		}
		printf("\n");
		disassemble_instruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
		#endif

		uint8_t instruction;
		switch (instruction = READ_BYTE()) {
			case OP_RETURN: {
				vm_pop();
				return INTERPRET_OK;
			}
			case OP_NEGATE: {
				vm_push(-vm_pop());
				break;
			}
			case OP_ADD: BINARY_OP(+); break;
			case OP_SUBTRACT: BINARY_OP(-); break;
			case OP_MULTIPLY: BINARY_OP(*); break;
			case OP_DIVIDE: BINARY_OP(/); break;
			case OP_CONSTANT: {
				Value constant = READ_CONSTANT();
				vm_push(constant);
				break;
			}
		}
	}

	#undef BINARY_OP
#undef READ_BYTE
#undef READ_CONSTANT
}


InterpretResult vm_interpret(const char* source) {
	compile(source);
	return INTERPRET_OK;
}


void vm_push(Value value) {
	*vm.stack_top = value;
	vm.stack_top++;
}


Value vm_pop() {
	vm.stack_top--;
	return *vm.stack_top;
}

