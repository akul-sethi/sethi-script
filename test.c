#include <stdio.h>
#include <inttypes.h>
#include "table.h"
#include "value.h"


int main(int argc, const char* argv[]) {
   ObjString* ptr = copyString("hello", 5);
//   Value val = MAKE_OBJ((Obj*)copyString("hello", 5));
//   printValue(val);
     printf(ptr->string, "%s");
     printf("%u", ptr->hash);
  return 0;
}