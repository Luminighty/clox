#ifndef clox_value_h
#define clox_value_h


typedef double Value;

typedef struct {
	int capacity;
	int count;
	Value* values;
} ValueArray;


ValueArray value_array_create();
void value_array_write(ValueArray* array, Value value);
void value_array_free(ValueArray* array);

void value_print(Value value);


#endif // clox_value_h
