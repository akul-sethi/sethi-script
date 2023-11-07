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

Compiler* current;
Parser parser;
Chunk* compilingChunk;

//Initializes the current Compiler with no locals and 0 depth
void initCompiler(Compiler* c) {
    c->currentScope = 0;
    c->localCount = 0;
    current = c;
}
//Moves the parser down one;
static void advance() {
    parser.previous = parser.current;
    parser.current = scanToken();
}

//Checks if the current token is the given type, and advances the parser if it is.
static bool match(TokenType type) {
    if(parser.current.type == type) {
        advance();
        return true;
    }
    return false;
}

//Returns true if the current token has given type. Does not advance parser.
static bool check(TokenType type) {
    return parser.current.type == type;
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

//Consumes the given type at the current slot of the parser and throws error if it does not exist
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

static void expressionStatement();
static void printStatement();
static void statement();
static void declaration();

//What to do when entering block
static void enterBlock() {
   current->currentScope++;
}
//Parse the block
static void block() {
    while(!check(TOKEN_RIGHT_CURLY) && !check(TOKEN_EOF)) {
        declaration();
    }
}

//What to do when exiting block
static void exitBlock() {
    consume(TOKEN_RIGHT_CURLY, "Expects a closing }");
    current->currentScope--;
    while(current->localCount > 0 && current->locals[current->localCount - 1].depth > current->currentScope) {
        current->localCount--;
        emitByte(OP_POP, parser.current.line);
    }
}

//Parse block statement
static void blockStatement() {
    enterBlock();
    block();
    exitBlock();
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

//Parses a string
static void string(bool canAssign) {
    int line = parser.previous.line;
    Value val = {.type = VALUE_OBJ, .as.obj = (Obj*)copyString(parser.previous.start + 1,
    parser.previous.length - 2)};
    int index = addConstant(compilingChunk, val);
    emitBytes(OP_CONSTANT, index, line);
}
 
//Returns true if the given token and this one represent the same identifer
static bool sameIdentifier(Token* one, Token* two) {
    if(one->type != TOKEN_IDENTIFIER || two->type != TOKEN_IDENTIFIER) {
        return false;
    } else {
        return one->length == two->length && memcmp(one->start, two->start, one->length) == 0;
    }
}

//Returns the index of the local on the stack which is equivalent to the given token. Returns -1 if none exist.
static int getLocal(Token* token) {
    for(int i = current->localCount - 1; i >= 0; i--) {
        if(current->locals[i].depth != -1 && sameIdentifier(&current->locals[i].token, token)) {
            return i;
        }
    }
    return -1;
}

//Parses a variable
static void variable(bool canAssign) {
    int index = getLocal(&parser.previous);
    OpCode setOp;
    OpCode getOp;
    if(index == -1) {
        index = addConstant(compilingChunk, (Value){.type = VALUE_OBJ, .as.obj = (Obj*)copyString(parser.previous.start,
        parser.previous.length)});
        setOp = OP_SET_GLOB;
        getOp = OP_GET_GLOB;
    } else {
        setOp = OP_SET_LOC;
        getOp = OP_GET_LOC;
    }


    if(canAssign && match(TOKEN_EQUAL)) {
        expression();
        emitBytes(setOp, index, parser.previous.line);
    } else {
        emitBytes(getOp, index, parser.previous.line);
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
        if(infix == NULL) {
            errorAtToken(&parser.previous, "No infix operator associated with token");
            return;
        }

        infix(canAssign);
    }

    if(canAssign && match(TOKEN_EQUAL)) {
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



//Defines variable
static void definition(int index) {
    if(current->currentScope == 0) {
        emitBytes(OP_DEFINE_GLOB, index, parser.current.line);
    } else {
        current->locals[current->localCount - 1].depth = current->currentScope;
    }
}

//Declares and defines local and global variables
static void varDeclaration() {
    consume(TOKEN_IDENTIFIER, "Expect identifier");
    int index;

    //Declares variable
    if(current->currentScope > 0) {
        Local newLocal;
        newLocal.token = parser.previous;
        newLocal.depth = -1;
        for(int i = current->localCount - 1; i >= 0; i--) {
            if(current->locals[i].depth < current->currentScope) {
                break;
            }
            if(sameIdentifier(&newLocal.token, &current->locals[i].token)) {
                errorAtToken(&newLocal.token, "Cannot declare the same variable twice in the same scope");
                return;
            }
        }
        current->locals[current->localCount] = newLocal;
        current->localCount++;
    } else {
        //Creates string for global variable (part of defining; put here for succictness)
        index = addConstant(compilingChunk, (Value){.type = VALUE_OBJ, .as.obj = (Obj*) copyString(parser.previous.start, parser.previous.length)});
    }

    if(match(TOKEN_EQUAL)) {
        expression();
    } else {
        emitByte(OP_NIL, parser.current.line);
    }
    definition(index);
    consume(TOKEN_SEMI, "Expected semicolon");
   
}


void statement() {

    if(match(TOKEN_PRINT)) {
        printStatement();
    } else if(match(TOKEN_LEFT_CURLY)) {
        blockStatement();
    } else {
        expressionStatement();
    }       
}


void declaration() {
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