#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "../table.h"
#include "../value.h"
#include "../vm.h"
#include <assert.h>

//Tests hash table implementation
int main(int argc, const char* argv[]) {
Table t;
initTable(&t);
initVM();

//Table getters and setters
assert(t.count == 0);
assert(t.capacity == 0);

// ObjString* key1 = copyString("key1", 4);
// ObjString* key2 = copyString("key2", 4);

uint32_t key1_hash = hash("key1", 4);
uint32_t key2_hash = hash("key2", 4);
ObjString key1 = (ObjString){.hash = key1_hash, .length = 4, .string = "key1"};
ObjString key2 = (ObjString) {.hash = key2_hash, .length = 4, .string = "key2"};



set(&t, &key1, MAKE_BOOL(true));
set(&t, &key2, MAKE_BOOL(false));

assert(t.count == 2);
assert(t.capacity == 8);

Value val1 = *get(&t, &key1);
Value val2 = *get(&t, &key2);

assert(val1.as.boolean == true);
assert(val2.as.boolean == false);

//Table Grow
Table t3;
initTable(&t3);
ObjString key3 = (ObjString){.hash = hash("key3", 4), .length = 4, .string = "key3"};
ObjString key4 = (ObjString) {.hash = hash("key4", 4), .length = 4, .string = "key4"};
ObjString key5 = (ObjString){.hash = hash("key5", 4), .length = 4, .string = "key5"};
ObjString key6 = (ObjString) {.hash = hash("key6", 4), .length = 4, .string = "key6"};
ObjString key7 = (ObjString) {.hash = hash("key7", 4), .length = 4, .string = "key7"};
ObjString key8 = (ObjString) {.hash = hash("key8", 4), .length = 4, .string = "key8"};
ObjString key9 = (ObjString) {.hash = hash("key9", 4), .length = 4, .string = "key9"};

set(&t3, &key1, MAKE_BOOL(true));
set(&t3, &key2, MAKE_BOOL(true));
set(&t3, &key3, MAKE_BOOL(true));
set(&t3, &key4, MAKE_BOOL(true));
set(&t3, &key5, MAKE_BOOL(true));
set(&t3, &key6, MAKE_BOOL(true));
set(&t3, &key7, MAKE_BOOL(true));
set(&t3, &key8, MAKE_BOOL(true));
set(&t3, &key9, MAKE_BOOL(true));

assert(t3.count == 9);
assert(t3.capacity == 16);


//Table findStringInTable()
Table t2;
initTable(&t2);

ObjString* key4_string = findStringInTable(&t2, "key4", 4, hash("key4", 4));
assert(key4_string == NULL);

assert(findStringInTable(&t, "key1", 4, key1_hash) == &key1);
assert(findStringInTable(&t, "key2", 4, key2_hash) == &key2);

ObjString* key3_string = findStringInTable(&t, "key3", 4, hash("key3", 4));
assert(key3_string == NULL);
}