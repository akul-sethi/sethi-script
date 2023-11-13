# sethi-script
Efficient, lightweight language with a novel form of types: Bounded Types.

# Language Design

# Grammar
statement      → exprStmt ;  
               | ifStmt    
               | printStmt ;   
               | returnStmt  
               | whileStmt    
               | block    
glob_declaration    →   
               | funDecl      
               | varDecl ;    
               | statement    
local_declaration    →   
               | varDecl ;    
               | statement   
returnStmt     → return ;
               | return expr;  

funDecl        → def NAME block  
               | def NAME "{" local_declaration* "}"  
        
varDecl        → var NAME expr  
               | var NAME  

block          → "{" declaration* "}"    
