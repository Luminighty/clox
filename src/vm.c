#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "vm.h"


VM vm;

static void reset_stack() {
	vm.stack_top = vm.stack;
}


void vm_create() {
	reset_stack();
	vm.objects = NULL;
	vm.strings = table_create();
	vm.globals = table_create();
}


void vm_free() {
	table_free(&vm.strings);
	table_free(&vm.globals);
	objects_free();
}

static Value peek(int distance) {
	return vm.stack_top[-1 - distance];
}


static void error_runtime(const char* format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fputs("\n", stderr);

	size_t instruction = vm.ip - vm.chunk->code - 1;
	int line = vm.chunk->lines[instruction];
	fprintf(stderr, "[line %d] in script\n", line);
	reset_stack();
}

static bool is_falsy(Value value) {
	return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}


static void concatenate() {
	ObjString* b = AS_STRING(vm_pop());
	ObjString* a = AS_STRING(vm_pop());

	int length = a->length + b->length;
	char* chars = ALLOCATE(char, length + 1);
	memcpy(chars, a->chars, a->length);
	memcpy(chars + a->length, b->chars, b->length);
	chars[length] = '\0';

	ObjString* result = take_string(chars, length);
	vm_push(VALUE_OBJECT(result));
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define READ_STRING() (AS_STRING(READ_CONSTANT()))
#define BINARY_OP(value_type, op) \
	do { \
		if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
			error_runtime("Operands must be numbers."); \
			return INTERPRET_RUNTIME_ERROR; \
		} \
		double b = AS_NUMBER(vm_pop()); \
		double a = AS_NUMBER(vm_pop()); \
		vm_push(value_type(a op b)); \
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
				return INTERPRET_OK;
			}
			case OP_NEGATE: {
				if (!IS_NUMBER(peek(0))) {
					error_runtime("Operand must be a number.");
					return INTERPRET_RUNTIME_ERROR;
				}
				vm_push(VALUE_NUMBER(-AS_NUMBER(vm_pop())));
				break;
			}
			case OP_NOT:
				vm_push(VALUE_BOOL(is_falsy(vm_pop())));
				break;
			case OP_ADD: {
				if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
					concatenate();
				} else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
					double b = AS_NUMBER(vm_pop());
					double a = AS_NUMBER(vm_pop());
					vm_push(VALUE_NUMBER(a + b));
				} else {
					error_runtime("Operands must be two numbers or two strings.");
					return INTERPRET_RUNTIME_ERROR;
				}
				break;
			}
			case OP_SUBTRACT: BINARY_OP(VALUE_NUMBER, -); break;
			case OP_MULTIPLY: BINARY_OP(VALUE_NUMBER, *); break;
			case OP_DIVIDE: BINARY_OP(VALUE_NUMBER, /); break;
			case OP_CONSTANT: {
				Value constant = READ_CONSTANT();
				vm_push(constant);
				break;
			}
			case OP_NIL: vm_push(VALUE_NIL); break;
			case OP_TRUE: vm_push(VALUE_BOOL(true)); break;
			case OP_FALSE: vm_push(VALUE_BOOL(false)); break;
			case OP_EQUAL: {
				Value b = vm_pop();
				Value a = vm_pop();
				vm_push(VALUE_BOOL(value_equal(a, b)));
				break;
			}
			case OP_GREATER: BINARY_OP(VALUE_NUMBER, >); break;
			case OP_LESS: BINARY_OP(VALUE_NUMBER, <); break;
			case OP_GET_GLOBAL: {
				ObjString* name = READ_STRING();
				Value value;
				if (!table_get(&vm.globals, name, &value)) {
					error_runtime("Undefined variable '%s'", name->chars);
					return INTERPRET_RUNTIME_ERROR;
				}
				vm_push(value);
				break;
			}
			case OP_DEFINE_GLOBAL: {
				ObjString* name = READ_STRING();
				table_insert(&vm.globals, name, peek(0));
				vm_pop();
				break;
			}
			case OP_SET_GLOBAL: {
				ObjString* name = READ_STRING();
				if (table_insert(&vm.globals, name, peek(0))) {
					table_delete(&vm.globals, name);
					error_runtime("Undefined variable '%s'.", name->chars);
					return INTERPRET_RUNTIME_ERROR;
				}
				break;
			}
			case OP_POP: vm_pop(); break;
			case OP_PRINT: {
				value_print(vm_pop());
				printf("\n");
				break;
			}
		}
	}

#undef BINARY_OP
#undef READ_STRING
#undef READ_BYTE
#undef READ_CONSTANT
}


InterpretResult vm_interpret(const char* source) {
	Chunk chunk = chunk_create();

	if (!compile(source, &chunk)) {
		chunk_free(&chunk);
		return INTERPRET_COMPILE_ERROR;
	}

	vm.chunk = &chunk;
	vm.ip = vm.chunk->code;

	InterpretResult result = run();

	chunk_free(&chunk);

	return result;
}


void vm_push(Value value) {
	*vm.stack_top = value;
	vm.stack_top++;
}


Value vm_pop() {
	vm.stack_top--;
	return *vm.stack_top;
}

