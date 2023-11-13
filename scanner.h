#ifndef sethi_scanner_h
#define sethi_scanner_h

typedef enum {
    //Single characters
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN, TOKEN_LEFT_CURLY, TOKEN_RIGHT_CURLY,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_SEMI, TOKEN_PLUS, TOKEN_MINUS, TOKEN_SLASH, TOKEN_STAR,
    

    //One or two characters
    TOKEN_BANG, TOKEN_LESS, TOKEN_GREATER, TOKEN_EQUAL,
    TOKEN_BANG_EQUAL, TOKEN_LESS_EQUAL, TOKEN_GREATER_EQUAL,
    TOKEN_EQUAL_EQUAL,

    //Literals
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

    //Keywords
    TOKEN_IF, TOKEN_FOR, TOKEN_WHILE, TOKEN_TRUE, TOKEN_FALSE, TOKEN_PRINT,
    TOKEN_NIL, TOKEN_VAR, TOKEN_ELSE, TOKEN_AND, TOKEN_OR, TOKEN_DEF, TOKEN_RETURN,

    //Special
    TOKEN_ERROR, TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;

void initScanner(const char* source);
Token scanToken();

#endif