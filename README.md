# sethi-script
Efficient, lightweight language with a novel form of types: Bounded Types.

# Language Design

# Grammar
A program is global_declaration*

statement      → exprStmt ;  
               | ifStmt    
               | printStmt ;   
               | returnStmt  
               | whileStmt    
               | block    
global_declaration    →   
               | funDecl      
               | varDecl ;    
               | statement   
               | structDecl  
local_declaration    →   
               | varDecl ;    
               | statement   
returnStmt     → return ;
               | return expr;  

funDecl        → def NAME (NAME*) block    
        
varDecl        → var NAME = expr  
               | var NAME  

block          → "{" local_declaration* "}"   

structDecl     → struct NAME(NAME*) "{" varDecl* "}" 
