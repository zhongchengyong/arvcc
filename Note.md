# Note for Implementation
## Lexer
Implement a lexer which accept characters from stdin, and return the token for input characters.

## AST
Three types of AST is used for Kaleidscope:  
* ExprAst for expression. 
* PrototypeAst for prototype.
* FunctionAst for function.

### Improvement
* Use class Lexer to encapsulate rather than extern.


## Parser
* Parse AST from top to bottom.
> class Value: SSA(static single assignment) register or SSA value.
> The most distinct aspect of SSA values is that their value is computed as the related instruction executes, and it does not get a new value until (and if) the instruction re-executes

## IR
* Add CodeGen for AST
* Add CodeGen() function call in HandleExtern() for IR output


### Improvement
* Use visit pattern

## Optimization
* Constant folding
* CSE: Common Subexpression Elimination
### Pass
1. Whole module pass, can run at link time.
2. per-function pass, operate on a single function at time.


## Project
* Add LLVM dependencies for project. see https://llvm.org/docs/CMake.html#embedding-llvm-in-your-project