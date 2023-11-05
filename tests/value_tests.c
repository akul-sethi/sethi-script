#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../memory.h"
#include "../value.h"
#include "../table.h"
#include "../vm.h"
#include <assert.h>


int main(int argc, const char* argv[]) {
    initVM();
    ObjString* string1 = copyString("string1", 7);
    ObjString* string1_dup = copyString("string1", 7);
       ObjString* string1_dup1 = copyString("string1", 7);
          ObjString* string1_dup2 = copyString("string1", 7);
             ObjString* string1_dup3 = copyString("string1", 7);
    ObjString* string2 = copyString("string2", 7);

    assert(string1 == string1);
    assert(string1 == string1_dup);
    assert(string1_dup1 == string1_dup2);
    assert(string1_dup3 == string1_dup);
    assert(string1 != string2);
}