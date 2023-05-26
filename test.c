#include <stdio.h>

typedef struct {
    int x;
    double z;
} Holder;

typedef struct {
    int y;
} Other;

int main(int argc, const char* argv[]) {
   Holder hold;
   int x = (int*)&hold;
   printf("%s", hold);
}