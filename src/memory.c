#include "memory.h"
#include "vm.h"
#include <stdlib.h>
#include "object.h"

extern VM vm;

void* reallocate(void *pointer, size_t old_size, size_t new_size) {
	if (new_size == 0) {
		free(pointer);
		return NULL;
	}

	void* result = realloc(pointer, new_size);
	if (result == NULL)
		exit(1);

	return result;
}

static void object_free(Obj* object) {
	switch (object->type) {
	case OBJ_STRING: {
		ObjString* string = (ObjString*)object;
		FREE_ARRAY(char, string->chars, string->length);
		FREE(ObjString, object);
		break;
	}
	}
}

void objects_free() {
	Obj* object = vm.objects;
	while (object != NULL) {
		Obj* next = object->next;
		object_free(object);
		object = next;
	}
}
