#include "value.h"
#include "memory.h"
#include "object.h"
#include <stdio.h>
#include <string.h>


ValueArray value_array_create() {
	ValueArray array;
	array.values = NULL;
	array.capacity = 0;
	array.count = 0;
	return array;
}


void value_array_write(ValueArray* array, Value value) {
	if (array->capacity < array->count + 1) {
		int old_capacity = array->capacity;
		array->capacity = GROW_CAPACITY(old_capacity);
		array->values = GROW_ARRAY(Value, array->values, old_capacity, array->capacity);
	}

	array->values[array->count] = value;
	array->count++;
}


void value_array_free(ValueArray* array) {
	FREE_ARRAY(Value, array->values, array->capacity);
	array->values = NULL;
	array->capacity = 0;
	array->count = 0;
}

void value_print(Value value) {
	switch (value.type) {
	case VAL_BOOL:
		printf("%s", AS_BOOL(value) ? "true" : "false");
		break;
	case VAL_NIL:
		printf("nil"); break;
	case VAL_NUMBER:
		printf("%g", AS_NUMBER(value));
		break;
	case VAL_OBJ:
		object_print(value);
		break;
	}
}

bool value_equal(Value left, Value right) {
	if (left.type != right.type) return false;
	switch (left.type) {
	case VAL_BOOL: return AS_BOOL(left) == AS_BOOL(right);
	case VAL_NIL: return true;
	case VAL_NUMBER: return AS_NUMBER(left) == AS_NUMBER(right);
	case VAL_OBJ: return AS_OBJECT(left) == AS_OBJECT(right);
	default: return false;
	}
}
