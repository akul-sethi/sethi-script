#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void repl() {
  char line[1024];

  for (;;) {
    printf(">>>");
    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    interpret(line);
  }
}

static char *readFile(const char *path) {
  FILE *ptr = fopen(path, "rb");

  fseek(ptr, 0, SEEK_END);
  int length = ftell(ptr);
  char *buffer = (char *)malloc(length + 1);
  rewind(ptr);

  size_t bytesRead = fread(buffer, sizeof(char), length, ptr);
  buffer[bytesRead] = '\0';

  fclose(ptr);
  return buffer;
}

static void runFile(const char *path) {
  char *source = readFile(path);
  InterpretResult result = interpret(source);
  free(source);

  if (result == INTERPRET_COMPILE_ERROR)
    exit(65);
  if (result == INTERPRET_RUNTIME_ERROR)
    exit(77);
}

int main(int argc, const char *argv[]) {
  initVM();
  if (argc == 1) {
    repl();
  } else if (argc == 2) {
    runFile(argv[1]);
  } else {
    fprintf(stderr, "There was an error too many args");
  }
  freeVM();
}
