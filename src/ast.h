//
// Created by zhong on 7/4/21.
//

#ifndef ARVCC_SRC_AST_H_
#define ARVCC_SRC_AST_H_

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include <string>
#include <memory>
#include <vector>

// We have expression, prototype and function object in Kaleidoscope.
class ExprAST {
 public:
  virtual ~ExprAST() = default;;
  virtual llvm::Value *CodeGen() = 0;
};

// Number
class NumberExprAST : public ExprAST {
  double m_val;
 public:
  NumberExprAST(double val) : m_val(val) {}
  llvm::Value *CodeGen() override;
};

 class VariableExprAST : public ExprAST {
   std::string m_name;
  public:
   VariableExprAST(const std::string& name) : m_name(name) {}
   llvm::Value *CodeGen() override;
 };

class BinaryExprAST : public ExprAST {
  char m_op;
  std::unique_ptr<ExprAST> m_lhs, m_rhs;
 public:
  BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs) :
  m_op{op}, m_lhs{std::move(lhs)}, m_rhs{std::move(rhs)} {}
  llvm::Value *CodeGen() override;
};

class CallExprAST : public ExprAST {
  std::string m_callee;
  std::vector<std::unique_ptr<ExprAST>> m_args;
 public:
  CallExprAST(const std::string &callee, std::vector<std::unique_ptr<ExprAST>> args) :
  m_callee{callee}, m_args{std::move(args)} {}
  llvm::Value *CodeGen() override;
};

// Prototype
class PrototypeAST {
  std::string m_name;
  std::vector<std::string> m_ars;
 public:
  PrototypeAST(const std::string &name, std::vector<std::string> args) : m_name{name}, m_ars{args} {}
};

/**
 * In Kaleidoscope, functions are typed with just a count of their arguments.
 * Since all values are double precision floating point, the type of each argument doesn’t need to be stored anywhere.
 * In a more aggressive and realistic language, the “ExprAST” class would probably have a type field.
 */
class FunctionAST {
  std::unique_ptr<PrototypeAST> m_proto;
  std::unique_ptr<ExprAST> m_body;
 public:
  FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body) :
  m_proto{std::move(proto)}, m_body{std::move(body)} {}
};

#endif //ARVCC_SRC_AST_H_
