#ifndef sethi_compile_h
#define sethi_compile_h
#include "chunk.h"
#include "scanner.h"
#include "value.h"
//Represents a Precedence for an operation.
typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
    PREC_CALL,
    PREC_PRIMARY
} Precedence;

//Reprsents a function which parses tokens. CanAssign represents if the parsed expression returned by this function can be assigned to a value.
typedef void (*ParseFn)(bool canAssign);

//Represents a Local with a Token and depth
typedef struct {
    Token token;
    int depth;
} Local;

//Represents a Compiler object and stores a list of current Locals, number of current locals, and the current depth
typedef struct {
    Local locals[256];
    int localCount;
    int currentScope;
} Compiler;
//Represents how a certain token parses. Includes functions for when it is in a prefix as well as infix context and its precedence.
typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;


bool compile(const char* source, Chunk* chunk);
void parsePrecedence(Precedence precedence);
void expression();
void initCompiler(Compiler* current);
ParseRule* getRule(TokenType type);
// ObjString* makeObjString(const char* start, int length);


#endif