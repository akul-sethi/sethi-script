#include "scanner.h"
#include "common.h"
#include <string.h>

typedef struct {
    const char* source;
    const char* current;
    int line;
} Scanner;

Scanner scanner;

void initScanner(const char* source) {
    scanner.source = source;
    scanner.current = source;
    scanner.line = 1;
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}
static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static char next() {
    scanner.current++;
    return scanner.current[-1];
}

static char peek() {
    return *scanner.current;
}

static char peekNext() {
    return *(scanner.current + 1);
}

static void buildErrorToken(Token* token, const char* message) {
    token->type = TOKEN_ERROR;
    token->start = message;
    token->length = strlen(message);
}

static void buildNumber(Token* token) {
    token->type = TOKEN_NUMBER;
    for(;;) {
        char c = peek();
        if(isDigit(c)) {
            next();
            token->length++;
        } else {
            return;
        }
    }
}

static void buildString(Token* token) {
    token->type = TOKEN_STRING;
    for(;;) {
        token->length++;
        char c = next(); 
        
        if(c == '\0') {
            buildErrorToken(token, "Could not find string ending");
            return;
        }
        else if(c == '\n') {
            scanner.line++;
        } else if (c == '"') {
            return;
        }
    }
}

static void checkKeyword(Token* token, const char* name, TokenType type, int length) {
   for(;;) {
        char currentChar = peek();
        if(isDigit(currentChar) || isAlpha(currentChar) || currentChar == '_') {
            token->length++;
            next(); 
        } else {
            break;
        }
   }

    if(token->length == length && strncmp(name, token->start, length) == 0) {
        token->type = type;  
    }
    
}

static void skipWhitespace() {
    for(;;) {
        char c = peek();
        if(c == ' ' || c == '\t') {
            next();
        } else if(c == '\n') {
            next();
            scanner.line++;
        } else {
            return;
        }
    }
}

static void buildIdentifier(Token* token) {
    token->type = TOKEN_IDENTIFIER;
    const char* start = token->start;
    char nextChar = peek();
    switch(*start) 
    {
        case 'i': checkKeyword(token, "if", TOKEN_IF, 2); break;
        case 'p': checkKeyword(token, "print", TOKEN_PRINT, 5); break;
        case 'w': checkKeyword(token, "while", TOKEN_WHILE, 5); break;
        case 't': checkKeyword(token, "true", TOKEN_TRUE, 4); break;
        case 'n': checkKeyword(token, "nil", TOKEN_NIL, 3); break;
        case 'v': checkKeyword(token, "var", TOKEN_VAR, 3); break;
        case 'e': checkKeyword(token, "else", TOKEN_ELSE, 4); break;
        case 'a': checkKeyword(token, "and", TOKEN_AND, 3); break;
        case 'o': checkKeyword(token, "or", TOKEN_OR, 2); break;
        case 'f': 
            switch (nextChar)
            {
            case 'o': checkKeyword(token, "for", TOKEN_FOR, 3); break;
            case 'a': checkKeyword(token, "false", TOKEN_FALSE, 5); break;
            default: checkKeyword(token, "", TOKEN_IDENTIFIER, 0); break;
            }
            break;
        default: checkKeyword(token, "", TOKEN_IDENTIFIER, 0); break;
        
    } 


}



Token scanToken() {
    skipWhitespace();
    char c = next();
    Token token;
    token.start = scanner.current - 1;
    token.length = 1;
    token.line = scanner.line;

    if(isAlpha(c) || c == '_') {
        buildIdentifier(&token);
        return token;
    }

    if(isDigit(c)) {
        buildNumber(&token);
        return token;
    }

    if(c == '"') {
        buildString(&token);
        return token;
    }

    switch (c)
    {
    case '\0': token.type = TOKEN_EOF; break;
    case '(': token.type = TOKEN_LEFT_PAREN; break;
    case ')': token.type = TOKEN_RIGHT_PAREN; break;
    case '{': token.type = TOKEN_LEFT_CURLY; break;
    case '}': token.type = TOKEN_RIGHT_CURLY; break;
    case ',': token.type = TOKEN_COMMA; break;
    case '.': token.type = TOKEN_DOT; break;
    case ';': token.type = TOKEN_SEMI; break;
    case '+': token.type = TOKEN_PLUS; break;
    case '-': token.type = TOKEN_MINUS; break;
    case '/': token.type = TOKEN_SLASH; break;
    case '*': token.type = TOKEN_STAR; break;
    case '!': {
        char nextC = peek();
        if(nextC == '=') {
            token.type = TOKEN_BANG_EQUAL;
            token.length++;
            next();
        } else {
            token.type = TOKEN_BANG;
        }
        break;
    }
    case '<': {
       char nextC = peek();
        if(nextC == '=') {
            token.type = TOKEN_LESS_EQUAL;
            token.length++;
            next();
        } else {
            token.type = TOKEN_LESS;
        }
        break;
    }
    case '>':  {
       char nextC = peek();
        if(nextC == '=') {
            token.type = TOKEN_GREATER_EQUAL;
            token.length++;
            next();
        } else {
            token.type = TOKEN_GREATER;
        }
        break;
    }

    case '=': {
        char nextC = peek();
        if(nextC == '=') {
            token.type = TOKEN_EQUAL_EQUAL;
            token.length++;
            next();
        } else {
            token.type = TOKEN_EQUAL;
        }
        break;
    }
    
    default: {
        buildErrorToken(&token, "Token does not exist");
        break;
    }
    }

    return token;
}