#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"
#include <stdint.h>


typedef enum {
	OBJ_STRING
} ObjType;


struct Obj {
	ObjType type;
	struct Obj* next;
};


struct ObjString {
	Obj obj;
	int length;
	char* chars;
	uint32_t hash;
};


static inline bool object_is_type(Value value, ObjType type) {
	return IS_OBJECT(value) && AS_OBJECT(value)->type == type;
}


#define OBJ_TYPE(value) (AS_OBJECT(value)->type)
#define IS_STRING(value) object_is_type(value, OBJ_STRING)

#define AS_STRING(value) ((ObjString*)AS_OBJECT(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJECT(value))->chars)


ObjString* string_copy(const char* chars, int length);
ObjString* take_string(char* chars, int length);

void object_print(Value value);

#endif // clox_object_h
