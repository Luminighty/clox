#include "value.h"
#include "memory.h"
#include <stdio.h>


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
	printf("%g", value);
}
