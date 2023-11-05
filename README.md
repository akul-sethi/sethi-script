# sethi-script
Efficient, lightweight language with a novel form of types: Bounded Types.

# Language Design

# Grammar
Vision:
statement      → exprStmt
               | forStmt
               | ifStmt
               | printStmt
               | returnStmt
               | whileStmt
               | block ;
declaration    → classDecl
               | funDecl
               | varDecl
               | statement ;
Currently:
statement      → exprStmt
               | printStmt ;
               | block;

declaration    → varDecl
               | statement ;
block          → "{" declaration* "}" ;