#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

Table table_create() {
	Table table;
	table.count = 0;
	table.capacity = 0;
	table.entries = NULL;
	return table;
}


void table_free(Table *table) {
	FREE_ARRAY(Entry, table->entries, table->capacity);
	table->count = 0;
	table->capacity = 0;
	table->entries = NULL;
}


static Entry* entry_find(Entry* entries, int capacity, ObjString* key) {
	uint32_t index = key->hash % capacity;
	Entry* tombstone = NULL;

	for (;;) {
		Entry* entry = &entries[index];
		if (entry->key == NULL) {
			if (IS_NIL(entry->value)) {
				return (tombstone != NULL) ? tombstone : entry;
			} else {
				if (tombstone == NULL)
					tombstone = entry;
			}

		} else if (entry->key == key) {
			return entry;
		} 
		index = (index + 1) % capacity;
	}
}

static void adjust_capacity(Table* table, int capacity) {
	Entry* entries = ALLOCATE(Entry, capacity);
	for (int i = 0; i < capacity; i++) {
		entries[i].key = NULL;
		entries[i].value = VALUE_NIL;
	}

	table->count = 0;
	for (int i = 0; i < table->capacity; i++) {
		Entry* entry = &table->entries[i];
		if (entry->key == NULL) continue;

		Entry* dest = entry_find(entries, capacity, entry->key);
		dest->key = entry->key;
		dest->value = entry->value;
		table->count = 0;
	}

	FREE_ARRAY(Entry, table->entries, table->capacity);

	table->entries = entries;
	table->capacity = capacity;
}


bool table_insert(Table* table, ObjString* key, Value value) {
	if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
		int capacity = GROW_CAPACITY(table->capacity);
		adjust_capacity(table, capacity);
	}

	Entry* entry = entry_find(table->entries, table->capacity, key);
	bool is_new_key = entry->key == NULL;
	if (is_new_key && IS_NIL(entry->value))
		table->count++;

	entry->key = key;
	entry->value = value;
	return is_new_key;
}


void table_add_all(Table *from, Table *to) {
	for (int i = 0; i < from->capacity; i++) {
		Entry* entry = &from->entries[i];
		if (entry->key != NULL)
			table_insert(to, entry->key, entry->value);
	}
}


bool table_get(Table *table, ObjString *key, Value *value) {
	if (table->count == 0) return false;

	Entry* entry = entry_find(table->entries, table->capacity, key);
	if (entry->key == NULL) return false;

	*value = entry->value;
	return true;
}


bool table_delete(Table *table, ObjString *key) {
	if (table->count == 0) return false;

	Entry* entry = entry_find(table->entries, table->capacity, key);
	if (entry->key == NULL) return false;

	entry->key = NULL;
	entry->value = VALUE_BOOL(true);
	return true;

}


ObjString* table_find_string(Table *table, const char *chars, int length, uint32_t hash) {
	if (table->count == 0) return NULL;

	uint32_t index = hash % table->capacity;
	for (;;) {
		Entry* entry = &table->entries[index];
		if (entry->key == NULL) {
			if (IS_NIL(entry->value)) return NULL;
		} else if (
			entry->key->length == length && 
			entry->key->hash == hash && 
			memcmp(entry->key->chars, chars, length) == 0
		) {
			return entry->key;
		}
		index = (index + 1) % table->capacity;
	}

}

