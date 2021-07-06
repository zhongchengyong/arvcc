//
// Created by zhong on 7/4/21.
//
#include "ast.h"
#include "logger.h"

#include "llvm/IR/Value.h"

#include <cstdio>


std::unique_ptr<ExprAST> LogError(const char *str) {
  fprintf(stderr, "LogError: %s\n", str);
  return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *str) {
  LogError(str);
  return nullptr;
}

llvm::Value *LogErrorV(const char *str) {
  LogError(str);
  return nullptr;
}