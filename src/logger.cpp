//
// Created by zhong on 7/4/21.
//

#include <cstdio>

#include "ast.h"
#include "logger.h"

std::unique_ptr<ExprAST> LogError(const char *str) {
  fprintf(stderr, "LogError: %s\n", str);
  return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *str) {
  LogError(str);
  return nullptr;
}
