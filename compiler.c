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
    bool panicMode;
} Parser;

Parser parser;
Chunk* compilingChunk;

//Moves the parser down one;
static void advance() {
    parser.previous = parser.current;
    parser.current = scanToken();
}

//Prints what line and token the error is on as well as a message
static void errorAtToken(Token* token, const char* message) {
    if(parser.panicMode) {
        return;
    }
    parser.panicMode = true;
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

//Advances the parser until it gets to a statement boundry and turns panic mode off
static void synchronize() {
    parser.panicMode = false;
    while(parser.current.type != TOKEN_EOF) {
        if(parser.previous.type == TOKEN_SEMI) return;
        
        switch(parser.previous.type) {
            case TOKEN_FOR: return;
            case TOKEN_WHILE: return;
            case TOKEN_PRINT: return;
            case TOKEN_IF: return;
            case TOKEN_VAR: return;
            default: break;
        }
        advance();
    }
}

//Consumes the given type and throws error if it does not exist
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


//Pushes a constant op, adds a constant to the pool, and adds its index afterwards
static void constant(bool canAssign) {
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

//Checks if the current token is the given type, and advances the parser.
static bool match(TokenType type) {
    if(parser.current.type == type) {
        advance();
        return true;
    }
    return false;
}

//Parses and expression and consumes a final parenthesis
static void grouping(bool canAssign) {
   expression();
   consume(TOKEN_RIGHT_PAREN, "Expects a ')'");
}

static void unary(bool canAssign) {
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


static void binary(bool canAssign) {
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

static void string(bool canAssign) {
    int line = parser.previous.line;
    Value val = {.type = VALUE_OBJ, .as.obj = (Obj*)copyString(parser.previous.start + 1,
    parser.previous.length - 2)};
    int index = addConstant(compilingChunk, val);
    emitBytes(OP_CONSTANT, index, line);
}

static void variable(bool canAssign) {
    int index = addConstant(compilingChunk, (Value){.type = VALUE_OBJ, .as.obj = (Obj*)copyString(parser.previous.start,
    parser.previous.length)});
    if(canAssign && match(TOKEN_EQUAL)) {
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

//Parses until it gets to a token which has less than or equal precedence than the given precedence
void parsePrecedence(Precedence precedence){
    advance();

    ParseFn prefix = getRule(parser.previous.type)->prefix;

    if(prefix == NULL) {
        errorAtToken(&parser.previous, "Requires expression after prefix");
        return;
    }

    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefix(canAssign);


    while(precedence <= getRule(parser.current.type)->precedence) {
        advance();

        ParseFn infix = getRule(parser.previous.type)->infix;

        infix(canAssign);
    }

    if(canAssign && match(TOKEN_EQUAL)) {
        errorAtToken(&parser.current, "Illegal assignment target");
    }
}

static void printStatement() {
    expression();
    consume(TOKEN_SEMI, "Expected semicolon");
    emitByte(OP_PRINT, parser.current.line);
}

static void expressionStatement() {
    expression();
    consume(TOKEN_SEMI, "Expected semicolon");
    emitByte(OP_POP, parser.current.line);
}

static void varDeclaration() {
    consume(TOKEN_IDENTIFIER, "Expect identifier");
    int index = addConstant(compilingChunk, (Value){.type = VALUE_OBJ, .as.obj = (Obj*) copyString(parser.previous.start, parser.previous.length)});

    if(match(TOKEN_EQUAL)) {
        expression();
    } else {
        emitByte(OP_NIL, parser.current.line);
    }
    consume(TOKEN_SEMI, "Expected semicolon");
    emitBytes(OP_DEFINE_GLOB, index, parser.current.line);
}


static void statement() {

    if(match(TOKEN_PRINT)) {
        printStatement();
    } else {
        expressionStatement();
    }       
}


static void declaration() {
    if(match(TOKEN_VAR)) {
        varDeclaration();
    } else {
        statement();
    }
     if(parser.panicMode) synchronize();
}

void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

bool compile(const char* source, Chunk* chunk) {
    initScanner(source);
    parser.hadError = false;
    parser.panicMode = false;
    compilingChunk = chunk;

    advance();
    while(!match(TOKEN_EOF)) {
        declaration();
    }
    
    endCompile(parser.current.line);
    return !parser.hadError;
}