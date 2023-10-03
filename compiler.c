#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"
#include "chunk.h"
#include <stdlib.h>
#include <string.h>
#include "vm.h"
#include "debug.h"

typedef struct {
    Token previous;
    Token current;
    bool hadError;
} Parser;

Parser parser;
Chunk* compilingChunk;


static void advance() {
    parser.previous = parser.current;
    parser.current = scanToken();
}

static void errorAtToken(Token* token, const char* message) {
    if(parser.hadError) {
        return;
    }
    printf("Error on line, %d, at ", token->line);
    if(token->type == TOKEN_EOF) {
        printf("end of file ");
    } else if(token->type == TOKEN_ERROR) {
        
    } else {
        printf("%.*s: ", token->length, token->start);
    }

    printf("%s\n", message);
    parser.hadError = true;
}

static void consume(TokenType type, const char* message) {
    if(parser.current.type == type) {
        advance();
        return;
    }

    errorAtToken(&parser.current, message);
}

static Chunk* currentChunk() {
    return compilingChunk;
}

static void emitByte(uint8_t byte, int line) {
    Chunk* chunk = currentChunk();
    writeChunk(chunk, byte, line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2, int line) {
   emitByte(byte1, line);
   emitByte(byte2, line);
} 

static void emitReturn(int line) {
    emitByte(OP_RETURN, line);
}

static void endCompile(int line) {
    emitReturn(line);
}



static void constant() {
    int line = parser.previous.line;

    switch (parser.previous.type)
    {
    case TOKEN_TRUE: emitByte(OP_TRUE, line); break;
    case TOKEN_FALSE: emitByte(OP_FALSE, line); break;
    case TOKEN_NIL: emitByte(OP_NIL, line); break;
    default: {
            Value val = MAKE_NUM(atoi(parser.previous.start));
            int index = addConstant(currentChunk(), val);
            emitBytes(OP_CONSTANT, index, line);
        }
    }

}

static bool match(TokenType type) {
    if(parser.current.type == type) {
        advance();
        return true;
    }
    return false;
}

static void grouping() {
   expression();
   consume(TOKEN_RIGHT_PAREN, "Expects a ')'");
}

static void unary() {
    TokenType type = parser.previous.type;
    int line = parser.previous.line;

    parsePrecedence(PREC_UNARY);

    switch (type)
    {
    case TOKEN_MINUS: emitByte(OP_NEGATE, line); break;
    case TOKEN_BANG: emitByte(OP_FALSIFY, line); break;
    default: break;
    }
}


static void binary() {
    TokenType type = parser.previous.type;
    int line = parser.previous.line;
    ParseRule* rule = getRule(type);

    parsePrecedence((Precedence)(rule->precedence + 1));

    switch (type)
    {
    case TOKEN_PLUS: emitByte(OP_ADD, line); break;
    case TOKEN_MINUS: emitByte(OP_SUBTRACT, line); break;
    case TOKEN_SLASH: emitByte(OP_DIVIDE, line); break;
    case TOKEN_STAR: emitByte(OP_MUL, line); break;
    case TOKEN_EQUAL_EQUAL: emitByte(OP_EQUALITY, line); break;
    case TOKEN_LESS: emitByte(OP_LESS, line); break;
    case TOKEN_GREATER: emitByte(OP_GREATER, line); break;
    case TOKEN_GREATER_EQUAL: emitByte(OP_GREATER_EQUAL, line); break;
    case TOKEN_LESS_EQUAL: emitByte(OP_LESS_EQUAL, line); break;
    default: break;
    }
}
 ObjString* makeObjString(const char* start, int length) {
    char* heapString = (char*) malloc(length  + 1);
    memcpy(heapString, start, length);
    heapString[length] = '\0';

    ObjString* objString = (ObjString*) malloc(sizeof(ObjString));
    objString->obj.type = OBJ_STRING;
    objString->obj.next = vm.objects;
    objString->length = length;
    objString->string = heapString;

    
    vm.objects = &objString->obj;

    return objString;
}
static void string() {
    int line = parser.previous.line;
    Value val = {.type = VALUE_OBJ, .as.obj = (Obj*)makeObjString(parser.previous.start + 1,
    parser.previous.length - 2)};
    int index = addConstant(compilingChunk, val);
    emitBytes(OP_CONSTANT, index, line);
}

static void variable() {
    int index = addConstant(compilingChunk, (Value){.type = VALUE_OBJ, .as.obj = (Obj*)makeObjString(parser.previous.start,
    parser.previous.length)});
    if(match(TOKEN_EQUAL)) {
        expression();
        emitBytes(OP_SET_GLOB, index, parser.previous.line);
    } else {
        emitBytes(OP_GET_GLOB, index, parser.previous.line);
    }
}

 ParseRule rules[] = {
    [TOKEN_LEFT_PAREN] = {grouping, NULL, PREC_CALL},
    [TOKEN_RIGHT_PAREN] = {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_CURLY] = {NULL, NULL, PREC_NONE},
    [TOKEN_RIGHT_CURLY] = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PREC_NONE},
    [TOKEN_DOT] = {NULL, NULL, PREC_NONE},
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
    [TOKEN_FOR] = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PREC_NONE},
    [TOKEN_TRUE] = {constant, NULL, PREC_PRIMARY},
    [TOKEN_FALSE] = {constant, NULL, PREC_PRIMARY},
    [TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
    [TOKEN_NIL] = {constant, NULL, PREC_PRIMARY},
    [TOKEN_ERROR] = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF] = {NULL, NULL, PREC_NONE}
};

ParseRule* getRule(TokenType type) {
    return &rules[type];
}


 void parsePrecedence(Precedence precedence){
    advance();

    ParseFn prefix = getRule(parser.previous.type)->prefix;

    if(prefix == NULL) {
        errorAtToken(&parser.previous, "Requires expression after prefix");
        return;
    }

    prefix();

    while(precedence <= getRule(parser.current.type)->precedence) {
        advance();

        ParseFn infix = getRule(parser.previous.type)->infix;

        infix();
    }

}



static void printStatement() {
    expression();
    emitByte(OP_PRINT, parser.current.line);
}

static void expressionStatement() {
    expression();
    emitByte(OP_POP, parser.current.line);
}

static void declaration() {
    consume(TOKEN_IDENTIFIER, "Expect identifier");
    int index = addConstant(compilingChunk, (Value){.type = VALUE_OBJ, .as.obj = (Obj*) makeObjString(parser.previous.start, parser.previous.length)});

    if(match(TOKEN_EQUAL)) {
        expression();
    } else {
        emitByte(OP_NIL, parser.current.line);
    }

    emitBytes(OP_DEFINE_GLOB, index, parser.current.line);
}


static void statement() {

    if(match(TOKEN_PRINT)) {
        printStatement();
    } else if(match(TOKEN_VAR)) {
        declaration();
    }       
    else {
       expressionStatement();
    }

    consume(TOKEN_SEMI, "Expected semicolon");
}
void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

bool compile(const char* source, Chunk* chunk) {
    initScanner(source);
    parser.hadError = false;
    compilingChunk = chunk;

    advance();
    while(!match(TOKEN_EOF)) {
        statement();
    }
    
    endCompile(parser.current.line);
    return !parser.hadError;
}