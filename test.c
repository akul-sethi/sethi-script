#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "table.h"
#include "value.h"
#include "vm.h"

int main(int argc, const char* argv[]) {
   initVM();
   ObjString* x = copyString("x", 1);
   ObjString* x2 = copyString("x", 1);
    ObjString* hello = copyString("hello", 5);
     ObjString* hello1 = copyString("hello", 5);
   printf("%d", x == x2);
   printf("%d", hello == hello1);
   


  return 0;
}