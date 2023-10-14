#ifndef sethi_compile_h
#define sethi_compile_h
#include "chunk.h"
#include "scanner.h"
#include "value.h"
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

typedef void (*ParseFn)();

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;


bool compile(const char* source, Chunk* chunk);
void parsePrecedence(Precedence precedence);
void expression();
ParseRule* getRule(TokenType type);
// ObjString* makeObjString(const char* start, int length);


#endif