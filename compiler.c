#include <stdio.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "scanner.h"
#include "vm.h"
#include <stdlib.h>
#include <string.h>

#define UINT_16_SIZE 65535

typedef struct {
  Token previous;
  Token current;
  bool hadError;
  bool panicMode;
} Parser;

Compiler *current;
Parser parser;
Chunk *compilingChunk;
Chunk *mainChunk;

// Initializes the current Compiler with no locals and 0 depth
void initCompiler(Compiler *c) {
  c->currentScope = 0;
  c->localCount = 0;
  current = c;
}
// Moves the parser down one;
static void advance() {
  parser.previous = parser.current;
  parser.current = scanToken();
}

// Checks if the current token is the given type, and advances the parser if it
// is.
static bool match(TokenType type) {
  if (parser.current.type == type) {
    advance();
    return true;
  }
  return false;
}

// Returns true if the current token has given type. Does not advance parser.
static bool check(TokenType type) { return parser.current.type == type; }

// Prints what line and token the error is on as well as a message
static void errorAtToken(Token *token, const char *message) {
  if (parser.panicMode) {
    return;
  }
  parser.panicMode = true;
  printf("Error on line, %d, at ", token->line);
  if (token->type == TOKEN_EOF) {
    printf("end of file ");
  } else if (token->type == TOKEN_ERROR) {

  } else {
    printf("%.*s: ", token->length, token->start);
  }

  printf("%s\n", message);
  parser.hadError = true;
}

// Advances the parser until it gets to a statement boundry and turns panic mode
// off
static void synchronize() {
  parser.panicMode = false;
  while (parser.current.type != TOKEN_EOF) {
    if (parser.previous.type == TOKEN_SEMI)
      return;

    switch (parser.previous.type) {
    case TOKEN_FOR:
      return;
    case TOKEN_WHILE:
      return;
    case TOKEN_PRINT:
      return;
    case TOKEN_STRUCT:
      return;
    case TOKEN_DEF:
      return;
    case TOKEN_IF:
      return;
    case TOKEN_VAR:
      return;
    default:
      break;
    }
    advance();
  }
}

// Consumes the given type at the current slot of the parser and throws error if
// it does not exist
static void consume(TokenType type, const char *message) {
  if (parser.current.type == type) {
    advance();
    return;
  }

  errorAtToken(&parser.current, message);
}

// Returns the current compiling chunk.
static Chunk *currentChunk() { return compilingChunk; }

// Sets the current compiling chunk
static void setCurrentChunk(Chunk *chunk) { compilingChunk = chunk; }

// Emits one OP.
static void emitByte(uint8_t byte, int line) {
  Chunk *chunk = currentChunk();
  writeChunk(chunk, byte, line);
}

// Emits two OPS.
static void emitBytes(uint8_t byte1, uint8_t byte2, int line) {
  emitByte(byte1, line);
  emitByte(byte2, line);
}

// Emits return OP.
static void emitReturn(int line) { emitByte(OP_RETURN, line); }

// Emits a jump instruction and two bytes as placeholders for the length of the
// jump.
static void emitJump(uint8_t op) {
  emitByte(op, parser.previous.line);
  emitByte('\xff', parser.previous.line);
  emitByte('\xff', parser.previous.line);
}
// Emits a jump back instruction and two bytes representing how far back to
// jump. Throws error if jump is larger than UINT_16_SIZE
/// @param backCount represents the count of the byte which ip will be set too.
static void emitJumpBack(int backCount) {
  // Add 3 to compensate for where the pointer is after processing both operands
  if (currentChunk()->count - backCount + 3 > UINT_16_SIZE) {
    errorAtToken(&parser.previous, "Loop is too large");
  }
  uint16_t jumpLength = (uint16_t)(currentChunk()->count - backCount + 3);
  uint8_t msb = (uint8_t)(jumpLength >> 8);
  uint8_t lsb = (uint8_t)jumpLength;
  emitByte(OP_JUMP_BACK, parser.previous.line);
  emitByte(msb, parser.previous.line);
  emitByte(lsb, parser.previous.line);
}

// Ends compile by emitting a return OP
static void endCompile(int line) { emitReturn(line); }

static void expressionStatement();
static void printStatement();
static void statement();
static void globalDeclaration();
static void localDeclaration();

// Add local to compiler
static void addLocal(Local local) {
  current->locals[current->localCount] = local;
  current->localCount++;
}

// What to do when entering block
static void enterBlock() { current->currentScope++; }
// Parse the block
static void block() {
  while (!check(TOKEN_RIGHT_CURLY) && !check(TOKEN_EOF)) {
    localDeclaration();
  }
}

// What to do when exiting block
static void exitBlock(bool emitPops) {
  current->currentScope--;
  while (current->localCount > 0 &&
         current->locals[current->localCount - 1].depth >
             current->currentScope) {
    current->localCount--;
    if (emitPops) {
      emitByte(OP_POP, parser.current.line);
    }
  }
}

// Parse block statement
static void blockStatement() {
  enterBlock();
  block();
  consume(TOKEN_RIGHT_CURLY, "Expects a closing }");
  exitBlock(true);
}

// Patches section of chunk starting the byte after "start" and encompassing two
// bytes total. Patches with value which would bring ip to current tip of chunk
// if the ip starts by pointing at second byte.
static void patchJump(int startCount) {

  if (currentChunk()->count - startCount - 2 > UINT_16_SIZE) {
    errorAtToken(&parser.previous, "Cannot jump that much code");
  }
  uint16_t jumpLength = currentChunk()->count - startCount - 2;
  uint8_t msb = (uint8_t)(jumpLength >> 8);
  uint8_t lsb = (uint8_t)jumpLength;
  currentChunk()->code[startCount] = msb;
  currentChunk()->code[startCount + 1] = lsb;
}

// Parses an if statement
static void ifStatement() {
  consume(TOKEN_LEFT_PAREN, "Needs '(' after if token");
  expression();
  consume(TOKEN_RIGHT_PAREN, "Needs closing ')' for if token");

  int skipIfJump = currentChunk()->count + 1;
  emitJump(OP_JUMP_IF_FALSE);
  emitByte(OP_POP, parser.previous.line);

  statement();

  int exitJumpCount = currentChunk()->count + 1;
  emitJump(OP_JUMP);
  patchJump(skipIfJump);

  if (match(TOKEN_ELSE)) {
    statement();
  }
  emitByte(OP_POP, parser.previous.line);
  patchJump(exitJumpCount);
}

// Parses while statement
static void whileStatement() {
  consume(TOKEN_LEFT_PAREN, "Needs '(' after if token");
  int beforeExpressionCount = currentChunk()->count;
  expression();
  consume(TOKEN_RIGHT_PAREN, "Needs closing ')' for if token");

  int skipWhileJump = currentChunk()->count + 1;
  emitJump(OP_JUMP_IF_FALSE);
  emitByte(OP_POP, parser.previous.line);
  statement();
  emitJumpBack(beforeExpressionCount);

  patchJump(skipWhileJump);
  emitByte(OP_POP, parser.previous.line);
}

// Pushes a constant op, adds a constant to the pool, and adds its index
// afterwards
static void constant(bool canAssign) {
  int line = parser.previous.line;

  switch (parser.previous.type) {
  case TOKEN_TRUE:
    emitByte(OP_TRUE, line);
    break;
  case TOKEN_FALSE:
    emitByte(OP_FALSE, line);
    break;
  case TOKEN_NIL:
    emitByte(OP_NIL, line);
    break;
  default: {
    Value val = MAKE_NUM(atoi(parser.previous.start));
    int index = addConstant(currentChunk(), val);
    emitBytes(OP_CONSTANT, index, line);
  }
  }
}

// Parses and expression and consumes a final parenthesis
static void grouping(bool canAssign) {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expects a ')'");
}

static void unary(bool canAssign) {
  TokenType type = parser.previous.type;
  int line = parser.previous.line;

  parsePrecedence(PREC_UNARY);

  switch (type) {
  case TOKEN_MINUS:
    emitByte(OP_NEGATE, line);
    break;
  case TOKEN_BANG:
    emitByte(OP_FALSIFY, line);
    break;
  default:
    break;
  }
}

static void binary(bool canAssign) {
  TokenType type = parser.previous.type;
  int line = parser.previous.line;
  ParseRule *rule = getRule(type);

  parsePrecedence((Precedence)(rule->precedence + 1));

  switch (type) {
  case TOKEN_PLUS:
    emitByte(OP_ADD, line);
    break;
  case TOKEN_MINUS:
    emitByte(OP_SUBTRACT, line);
    break;
  case TOKEN_SLASH:
    emitByte(OP_DIVIDE, line);
    break;
  case TOKEN_STAR:
    emitByte(OP_MUL, line);
    break;
  case TOKEN_EQUAL_EQUAL:
    emitByte(OP_EQUALITY, line);
    break;
  case TOKEN_LESS:
    emitByte(OP_LESS, line);
    break;
  case TOKEN_GREATER:
    emitByte(OP_GREATER, line);
    break;
  case TOKEN_GREATER_EQUAL:
    emitByte(OP_GREATER_EQUAL, line);
    break;
  case TOKEN_LESS_EQUAL:
    emitByte(OP_LESS_EQUAL, line);
    break;
  default:
    break;
  }
}

// Parses a string
static void string(bool canAssign) {
  int line = parser.previous.line;
  Value val = {.type = VALUE_OBJ,
               .as.obj = (Obj *)copyString(parser.previous.start + 1,
                                           parser.previous.length - 2)};
  int index = addConstant(compilingChunk, val);
  emitBytes(OP_CONSTANT, index, line);
}

// Returns true if the given token and this one represent the same identifer
static bool sameIdentifier(Token *one, Token *two) {
  if (one->type != TOKEN_IDENTIFIER || two->type != TOKEN_IDENTIFIER) {
    return false;
  } else {
    return one->length == two->length &&
           memcmp(one->start, two->start, one->length) == 0;
  }
}

// Returns the index of the local on the stack which is equivalent to the given
// token. Returns -1 if none exist.
static int getLocal(Token *token) {
  for (int i = current->localCount - 1; i >= 0; i--) {
    if (current->locals[i].depth != -1 &&
        sameIdentifier(&current->locals[i].token, token)) {
      return i;
    }
  }
  return -1;
}

// Parses a variable
static void variable(bool canAssign) {
  int index = getLocal(&parser.previous);
  OpCode setOp;
  OpCode getOp;
  if (index == -1) {
    index = addConstant(compilingChunk, (Value){.type = VALUE_OBJ,
                                                .as.obj = (Obj *)copyString(
                                                    parser.previous.start,
                                                    parser.previous.length)});
    setOp = OP_SET_GLOB;
    getOp = OP_GET_GLOB;
  } else {
    setOp = OP_SET_LOC;
    getOp = OP_GET_LOC;
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    expression();
    emitBytes(setOp, index, parser.previous.line);
  } else if (match(TOKEN_LEFT_PAREN)) {
    emitBytes(OP_NIL, OP_NIL, parser.previous.line);
    emitByte(OP_NIL, parser.previous.line);
    uint8_t numParams = 0;
    while (!match(TOKEN_RIGHT_PAREN) && !match(TOKEN_EOF) && !parser.hadError) {
      numParams++;
      expression();

      if (match(TOKEN_RIGHT_PAREN)) {
        break;
      }
      consume(TOKEN_COMMA, "Needs comma between variables");
    }
    emitBytes(getOp, index, parser.previous.line);
    emitBytes(OP_CALL, numParams, parser.previous.line);
  } else {
    emitBytes(getOp, index, parser.previous.line);
  }
}

// Parses AND operator skips rest of expression if first operand is false
static void and_(bool canAssign) {
  int jumpCount = currentChunk()->count + 1;
  emitJump(OP_JUMP_IF_FALSE);
  parsePrecedence(PREC_AND + 1);
  emitByte(OP_AND, parser.previous.line);
  patchJump(jumpCount);
}

// Parses OR operator and skips rest of expression if the first one is true
static void or_(bool canAssign) {
  int jumpTheJumpCount = currentChunk()->count + 1;
  emitJump(OP_JUMP_IF_FALSE);
  int jumpTheSecondPartCount = currentChunk()->count + 1;
  emitJump(OP_JUMP);
  patchJump(jumpTheJumpCount);
  parsePrecedence(PREC_OR + 1);
  emitByte(OP_OR, parser.previous.line);
  patchJump(jumpTheSecondPartCount);
}

// Parses dot expressions
static void namespace(bool canAssign) {
  if (match(TOKEN_IDENTIFIER)) {
    int index = addConstant(
        compilingChunk,
        (Value){.type = VALUE_OBJ,
                .as.obj = (Obj *)copyString(parser.previous.start,
                                            parser.previous.length)});
    emitBytes(OP_NAMESPACE, index, parser.previous.line);
  } else {
    errorAtToken(&parser.current, "Must be an identifer");
  }
}

ParseRule rules[] = {[TOKEN_LEFT_PAREN] = {grouping, NULL, PREC_CALL},
                     [TOKEN_RIGHT_PAREN] = {NULL, NULL, PREC_NONE},
                     [TOKEN_LEFT_CURLY] = {NULL, NULL, PREC_NONE},
                     [TOKEN_RIGHT_CURLY] = {NULL, NULL, PREC_NONE},
                     [TOKEN_COMMA] = {NULL, NULL, PREC_NONE},
                     [TOKEN_DOT] = {NULL, namespace, PREC_PRIMARY},
                     [TOKEN_SEMI] = {NULL, NULL, PREC_NONE},
                     [TOKEN_PLUS] = {NULL, binary, PREC_TERM},
                     [TOKEN_MINUS] = {unary, binary, PREC_TERM},
                     [TOKEN_SLASH] = {NULL, binary, PREC_FACTOR},
                     [TOKEN_STAR] = {NULL, binary, PREC_FACTOR},
                     [TOKEN_BANG] = {unary, NULL, PREC_NONE},
                     [TOKEN_LESS] = {NULL, binary, PREC_COMPARISON},
                     [TOKEN_GREATER] = {NULL, binary, PREC_COMPARISON},
                     [TOKEN_EQUAL] = {NULL, NULL, PREC_NONE},
                     [TOKEN_BANG_EQUAL] = {NULL, NULL, PREC_NONE},
                     [TOKEN_LESS_EQUAL] = {NULL, binary, PREC_COMPARISON},
                     [TOKEN_GREATER_EQUAL] = {NULL, binary, PREC_COMPARISON},
                     [TOKEN_EQUAL_EQUAL] = {NULL, binary, PREC_EQUALITY},
                     [TOKEN_IDENTIFIER] = {variable, NULL, PREC_PRIMARY},
                     [TOKEN_STRING] = {string, NULL, PREC_NONE},
                     [TOKEN_NUMBER] = {constant, NULL, PREC_PRIMARY},
                     [TOKEN_IF] = {NULL, NULL, PREC_NONE},
                     [TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
                     [TOKEN_AND] = {NULL, and_, PREC_AND},
                     [TOKEN_OR] = {NULL, or_, PREC_OR},
                     [TOKEN_DEF] = {NULL, NULL, PREC_NONE},
                     [TOKEN_RETURN] = {NULL, NULL, PREC_NONE},
                     [TOKEN_FOR] = {NULL, NULL, PREC_NONE},
                     [TOKEN_WHILE] = {NULL, NULL, PREC_NONE},
                     [TOKEN_TRUE] = {constant, NULL, PREC_PRIMARY},
                     [TOKEN_FALSE] = {constant, NULL, PREC_PRIMARY},
                     [TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
                     [TOKEN_NIL] = {constant, NULL, PREC_PRIMARY},
                     [TOKEN_ERROR] = {NULL, NULL, PREC_NONE},
                     [TOKEN_EOF] = {NULL, NULL, PREC_NONE}};

ParseRule *getRule(TokenType type) { return &rules[type]; }

// Advances past the current token and parses the expression as long as it has
// precedence greater than or equal to the given precedence.
void parsePrecedence(Precedence precedence) {
  advance();

  ParseFn prefix = getRule(parser.previous.type)->prefix;

  if (prefix == NULL) {
    errorAtToken(&parser.previous, "Requires expression after prefix");
    return;
  }

  bool canAssign = precedence <= PREC_ASSIGNMENT;
  prefix(canAssign);

  while (precedence <= getRule(parser.current.type)->precedence) {
    advance();

    ParseFn infix = getRule(parser.previous.type)->infix;
    if (infix == NULL) {
      errorAtToken(&parser.previous, "No infix operator associated with token");
      return;
    }

    infix(canAssign);
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    errorAtToken(&parser.current, "Illegal assignment target");
  }
}

void printStatement() {
  expression();
  consume(TOKEN_SEMI, "Expected semicolon");
  emitByte(OP_PRINT, parser.current.line);
}

void expressionStatement() {
  expression();
  consume(TOKEN_SEMI, "Expected semicolon");
  emitByte(OP_POP, parser.current.line);
}

// Parses a return statment
static void returnStatement() {
  if (!check(TOKEN_SEMI) && !check(TOKEN_EOF)) {
    expression();
  } else {
    emitByte(OP_NIL, parser.previous.line);
  }
  consume(TOKEN_SEMI, "Needs ';' to finish line");
  emitByte(OP_RETURN, parser.previous.line);
}

// Defines variable
static void definition(int index) {
  if (current->currentScope == 0) {
    emitBytes(OP_DEFINE_GLOB, index, parser.current.line);
  } else {
    current->locals[current->localCount - 1].depth = current->currentScope;
  }
}

// Parses parameters by having the compiler add locals and moving the parser so
// that it has the closing ')' in the previous slot. Returns number of
// parameters
static int parseParameters() {
  consume(TOKEN_LEFT_PAREN, "Expect '(' after function definition");
  enterBlock();
  int numParams = 0;

  while (!match(TOKEN_RIGHT_PAREN) && !match(TOKEN_EOF) && !parser.hadError) {
    consume(TOKEN_IDENTIFIER, "Needs identifier after '(' in function def");
    Local newLocal;
    numParams++;
    newLocal.token = parser.previous;
    newLocal.depth = current->currentScope;
    for (int i = current->localCount - 1; i >= 0; i--) {
      if (current->locals[i].depth < current->currentScope) {
        break;
      }
      if (sameIdentifier(&newLocal.token, &current->locals[i].token)) {
        errorAtToken(
            &newLocal.token,
            "Cannot declare the same variable twice in the same scope");
        return 0;
      }
    }
    addLocal(newLocal);
    if (match(TOKEN_RIGHT_PAREN)) {
      break;
    }
    consume(TOKEN_COMMA, "Needs commas between variables");
  }

  return numParams;
}

// Creates a callable in the vms table. Creates a new chunk and sets the
// compiling one to this.
static void createCallable() {
  consume(TOKEN_IDENTIFIER, "Expect identifier");
  ObjString *funcName =
      copyString(parser.previous.start, parser.previous.length);

  Chunk *chunk = (Chunk *)malloc(sizeof(Chunk));
  setCurrentChunk(chunk);

  int numParams = parseParameters();

  Value funcVal =
      (Value){.type = VALUE_OBJ, .as.obj = (Obj *)createFunc(chunk, numParams)};

  set(&vm.table, funcName, funcVal);
}

// Declares and defines local and global variables
static void varDeclaration() {
  consume(TOKEN_IDENTIFIER, "Expect identifier");
  int index;

  // Declares variable
  if (current->currentScope > 0) {
    Local newLocal;
    newLocal.token = parser.previous;
    newLocal.depth = -1;
    for (int i = current->localCount - 1; i >= 0; i--) {
      if (current->locals[i].depth < current->currentScope) {
        break;
      }
      if (sameIdentifier(&newLocal.token, &current->locals[i].token)) {
        errorAtToken(
            &newLocal.token,
            "Cannot declare the same variable twice in the same scope");
        return;
      }
    }
    addLocal(newLocal);
  } else {
    // Creates string for global variable (part of defining; put here for
    // succictness)
    index = addConstant(compilingChunk, (Value){.type = VALUE_OBJ,
                                                .as.obj = (Obj *)copyString(
                                                    parser.previous.start,
                                                    parser.previous.length)});
  }

  if (match(TOKEN_EQUAL)) {
    expression();
  } else {
    emitByte(OP_NIL, parser.current.line);
  }
  definition(index);
  consume(TOKEN_SEMI, "Expected semicolon");
}

// Compiles the function for constructor
static void constructDeclaration() {
  createCallable();
  consume(TOKEN_LEFT_CURLY, "Needs '{' after function def");
  enterBlock();
  uint8_t fields = 0;
  while (match(TOKEN_VAR)) {
    varDeclaration();
    fields += 1;
    Local last = current->locals[current->localCount - 1];
    Value identifier = (Value){
        .type = VALUE_OBJ,
        .as.obj = (Obj *)copyString(last.token.start, last.token.length)};
    int index = addConstant(currentChunk(), identifier);
    emitBytes(OP_CONSTANT, index, parser.previous.line);
    Local empty;
    empty.depth = current->currentScope;
    empty.token =
        (Token){.type = TOKEN_IDENTIFIER, .start = "", 0, parser.previous.line};
    addLocal(empty);
  }

  consume(TOKEN_RIGHT_CURLY, "Needs '}' to close the function");
  emitBytes(OP_TABLE, fields, parser.previous.line);
  emitByte(OP_RETURN, parser.previous.line);
  exitBlock(false);
  exitBlock(false);
  setCurrentChunk(mainChunk);
}

// Compiles the function for the predicate. Has form isNAME
static void predDeclaration() {}

// Compiles a struct; creates a constructor and predicate function in the heap;
// stores them in the vm table
static void structDeclaration() { constructDeclaration(); }

// Compiles a function and creates a function object in the heap, and stores it
// in the vms table
static void funcDeclaration() {
  createCallable();

  consume(TOKEN_LEFT_CURLY, "Expects '{' after function def");
  block();
  // Default return
  emitBytes(OP_NIL, OP_RETURN, parser.previous.line);
  consume(TOKEN_RIGHT_CURLY, "Expects '}' after function body");

  // Returns compilation to the main chunk
  setCurrentChunk(mainChunk);

  // Remove locals
  exitBlock(false);
}

void statement() {

  if (match(TOKEN_PRINT)) {
    printStatement();
  } else if (match(TOKEN_LEFT_CURLY)) {
    blockStatement();
  } else if (match(TOKEN_IF)) {
    ifStatement();
  } else if (match(TOKEN_RETURN)) {
    returnStatement();
  } else if (match(TOKEN_WHILE)) {
    whileStatement();
  } else {
    expressionStatement();
  }
}

void globalDeclaration() {
  if (match(TOKEN_DEF)) {
    funcDeclaration();
    if (parser.panicMode)
      synchronize();
  } else if (match(TOKEN_STRUCT)) {
    structDeclaration();
    if (parser.panicMode)
      synchronize();
  } else {
    localDeclaration();
  }
}

void localDeclaration() {
  if (match(TOKEN_VAR)) {
    varDeclaration();
  } else {
    statement();
  }
  if (parser.panicMode)
    synchronize();
}

void expression() { parsePrecedence(PREC_ASSIGNMENT); }

bool compile(const char *source, Chunk *chunk) {
  initScanner(source);
  parser.hadError = false;
  parser.panicMode = false;
  mainChunk = chunk;
  setCurrentChunk(mainChunk);

  advance();
  while (!match(TOKEN_EOF)) {
    globalDeclaration();
  }

  endCompile(parser.current.line);
  return !parser.hadError;
}
