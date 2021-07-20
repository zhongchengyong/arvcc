//
// Created by zhong on 7/4/21.
//

#include "parser.h"
#include "lexer.h"
#include "logger.h"
#include "token.h"

#include "llvm/Support/raw_os_ostream.h"

using namespace llvm;
using namespace llvm::orc;

extern int num_val;
extern int cur_tok;
extern std::string identifier_str;
extern std::unique_ptr<KaleidoscopeJIT> the_jit;
extern std::unique_ptr<Module> the_module;
extern std::unique_ptr<LLVMContext> the_context;

std::map<char, int> bin_op_precedence;
ExitOnError exit_on_err;

int GetTokenPrecedence() {
  if (!isascii(cur_tok)) return -1;

  int tok_prec = bin_op_precedence[cur_tok];
  if (tok_prec <= 0) return -1;
  return tok_prec;
}

std::unique_ptr<ExprAST> ParseNumberExpr() {
  auto result = std::make_unique<NumberExprAST>(num_val);
  GetNextToken();
  return std::move(result);
}

std::unique_ptr<ExprAST> ParseParenExpr() {
  GetNextToken(); // Eat '('
  auto v = ParseExpression();
  if (!v) return nullptr;

  if (cur_tok != ')')
    return LogError("expected ')'");
  GetNextToken();  // Eat ')'
  return v;
}

std::unique_ptr<ExprAST> ParseIdentifierExpr() {
  std::string id_name = identifier_str;

  GetNextToken();  // Eat identifier

  if (cur_tok != '(') return std::make_unique<VariableExprAST>(id_name);

  // Call
  GetNextToken();  // Eat '('
  std::vector<std::unique_ptr<ExprAST>> args;
  if (cur_tok != ')') {
    while (true) {
      if (auto arg = ParseExpression())
        args.push_back(std::move(arg));
      else
        return nullptr;
      if (cur_tok == ')')
        break;
      if (cur_tok != ',')
        return LogError("Expected ')' or ',' in argument list");
      GetNextToken();
    }
  }

  // Eat the ')'
  GetNextToken();
  return std::make_unique<CallExprAST>(id_name, std::move(args));
}

std::unique_ptr<ExprAST> ParsePrimary() {
  switch (cur_tok) {
    case tok_ident:return ParseIdentifierExpr();
    case tok_number:return ParseNumberExpr();
    case '(':return ParseParenExpr();
    default:return LogError("unknown token when expecting an expression");
  }
}

std::unique_ptr<ExprAST> ParseExpression() {
  auto lhs = ParsePrimary();
  if (!lhs) return nullptr;
  return ParseBinOpRHS(0, std::move(lhs));
}

std::unique_ptr<ExprAST> ParseBinOpRHS(int expr_prec, std::unique_ptr<ExprAST> lhs) {
  while (true) {
    int tok_prec = GetTokenPrecedence();

    // current token_prec is less than previous, we are done.
    if (tok_prec < expr_prec) return lhs;

    // Current token must be a bin_op, otherwise returned.
    int bin_op = cur_tok;
    GetNextToken(); // Eat bin_op

    auto rhs = ParsePrimary(); // primary will be eat here
    if (!rhs) return nullptr;

    int next_prec = GetTokenPrecedence();  // Now we have the next token
    if (tok_prec < next_prec) {
      // Use the rhs as lhs, prec + 1
      rhs = ParseBinOpRHS(tok_prec + 1, std::move(rhs));
      if (!rhs) return nullptr;
    }

    lhs = std::make_unique<BinaryExprAST>(bin_op, std::move(lhs), std::move(rhs));
  }
}

/// prototype
///   ::= id '(' id* ')'
std::unique_ptr<PrototypeAST> ParsePrototype() {
  if (cur_tok != tok_ident)
    return LogErrorP("Expected function name in prototype");

  std::string FnName = identifier_str;
  GetNextToken();

  if (cur_tok != '(')
    return LogErrorP("Expected '(' in prototype");

  std::vector<std::string> ArgNames;
  while (GetNextToken() == tok_ident)
    ArgNames.push_back(identifier_str);
  if (cur_tok != ')')
    return LogErrorP("Expected ')' in prototype");

  // success.
  GetNextToken(); // eat ')'.

  return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

/// definition ::= 'def' prototype expression
std::unique_ptr<FunctionAST> ParseDefinition() {
  GetNextToken(); // eat def.
  auto Proto = ParsePrototype();
  if (!Proto)
    return nullptr;

  if (auto E = ParseExpression())
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  return nullptr;
}

/// toplevelexpr ::= expression
std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
  if (auto E = ParseExpression()) {
    // Make an anonymous proto.
    auto Proto = std::make_unique<PrototypeAST>("__anon_expr",
                                                std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}

/// external ::= 'extern' prototype
std::unique_ptr<PrototypeAST> ParseExtern() {
  GetNextToken(); // eat extern.
  return ParsePrototype();
}

//===----------------------------------------------------------------------===//
// Top-Level parsing
//===----------------------------------------------------------------------===//

void HandleDefinition() {
  if (auto fn_ast = ParseDefinition()) {
    if (auto *fn_ir = fn_ast->CodeGen()) {
      fprintf(stderr, "Parsed a function definition.\n");
      fn_ir->print(errs());
      fprintf(stderr, "\n");
      exit_on_err(the_jit->addModule(
          ThreadSafeModule(std::move(the_module), std::move(the_context))));
      InitializeModuleAndPassManager();
    }
  } else {
    // Skip token for error recovery.
    GetNextToken();
  }
}

void HandleExtern() {
//  if (ParseExtern()) {
//    fprintf(stderr, "Parsed an extern\n");
//  } else {
//    // Skip token for error recovery.
//    GetNextToken();
//  }

  // Print IR info
  if (auto prototype_ast = ParseExtern()) {
    if (auto *fn_ir = prototype_ast->CodeGen()) {
      fprintf(stderr, "Read extern: ");
      fn_ir->print(errs());
      fprintf(stderr, "\n");
    }
  } else {
    // Skip token for error recovery.
    GetNextToken();
  }
}

void HandleTopLevelExpression() {
  // Evaluate a top-level expression into an anonymous function.
  if (ParseTopLevelExpr()) {
    fprintf(stderr, "Parsed a top-level expr\n");
  } else {
    // Skip token for error recovery.
    GetNextToken();
  }
}

/// top ::= definition | external | expression | ';'
void MainLoop() {
  while (true) {
    fprintf(stderr, "ready> ");
    switch (cur_tok) {
      case tok_eof:return;
      case ';': // ignore top-level semicolons.
        GetNextToken();
        break;
      case tok_def:HandleDefinition();
        break;
      case tok_extern:HandleExtern();
        break;
      default:HandleTopLevelExpression();
        break;
    }
  }
}