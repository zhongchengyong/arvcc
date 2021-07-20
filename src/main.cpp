//
// Created by zhong on 6/20/21.
//
#include "ast.h"
#include "lexer.h"
#include "parser.h"

#include <iostream>

extern std::map<char, int> bin_op_precedence;
extern std::unique_ptr<llvm::orc::KaleidoscopeJIT> the_jit;
extern llvm::ExitOnError exit_on_err;

int main() {
  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();
  LLVMInitializeNativeAsmParser();
  //------------------------Lexer Test-------------------------------------
//  std::cout << GetTok() << std::endl;
  //------------------------Lexer Test-------------------------------------

  //------------------------Parser Test-------------------------------------
  // Install standard binary operators.
  // 1 is lowest precedence.
  bin_op_precedence['<'] = 10;
  bin_op_precedence['+'] = 20;
  bin_op_precedence['-'] = 20;
  bin_op_precedence['*'] = 40; // highest.

  // Prime the first token.
  fprintf(stderr, "ready> ");
  GetNextToken();

  the_jit = exit_on_err(llvm::orc::KaleidoscopeJIT::Create());

  InitializeModuleAndPassManager();

  // Run the main "interpreter loop" now.
  MainLoop();

  return 0;
  //------------------------Parser Test-------------------------------------

}