#ifndef clox_value_h
#define clox_value_h

#include <stdbool.h>

typedef struct Obj Obj;
typedef struct ObjString ObjString;

typedef enum {
	VAL_NUMBER,
	VAL_BOOL,
	VAL_NIL,
	VAL_OBJ,
} ValueType;


typedef struct {
	ValueType type;
	union {
		double number;
		bool boolean;
		Obj* obj;
	} as;
} Value;


#define VALUE_BOOL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define VALUE_NIL ((Value){VAL_NIL, {.number = 0}})
#define VALUE_NUMBER(value) ((Value){VAL_NUMBER, {.number = value}})
#define VALUE_OBJECT(object) ((Value){VAL_OBJ, {.obj = (Obj*)object}})

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJECT(value) ((value).as.obj)

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJECT(value) ((value).type == VAL_OBJ)


typedef struct {
	int capacity;
	int count;
	Value* values;
} ValueArray;


ValueArray value_array_create();
void value_array_write(ValueArray* array, Value value);
void value_array_free(ValueArray* array);

bool value_equal(Value left, Value right);

void value_print(Value value);


#endif // clox_value_h
