#ifndef clox_table_h
#define clox_table_h

#include "value.h"
#include <stdbool.h>
#include <stdint.h>


typedef struct {
	ObjString* key;
	Value value;
} Entry;


typedef struct {
	int count;
	int capacity;
	Entry* entries;
} Table;


Table table_create();
void table_free(Table* table);
bool table_insert(Table* table, ObjString* key, Value value);
void table_add_all(Table* from, Table* to);
bool table_get(Table* table, ObjString* key, Value* value);
bool table_delete(Table* table, ObjString* key);

ObjString* table_find_string(Table* table, const char* chars, int length, uint32_t hash);


#endif // clox_table_h
