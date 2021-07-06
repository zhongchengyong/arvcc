//
// Created by zhong on 7/4/21.
//

#include "ast.h"
#include "logger.h"

using namespace llvm;

// TheContext is an opaque object that owns a lot of core LLVM data structures.
LLVMContext the_context;
// The Builder object is a helper object that makes it easy to generate LLVM instructions.
IRBuilder<> builder(the_context);
// TheModule is an LLVM construct that contains functions and global variables.
std::unique_ptr<Module> the_module;
// Symbol table for the code.
std::map<std::string, Value*> named_values;


llvm::Value *NumberExprAST::CodeGen() {
  return ConstantFP::get(the_context, APFloat(m_val));
}

llvm::Value *VariableExprAST::CodeGen() {
  // Look this variable up in the function.
  Value *V = named_values[m_name];
  if (!V)
    LogErrorV("Unknown variable name");
  return V;
}

llvm::Value *BinaryExprAST::CodeGen() {
  Value *l = m_lhs->CodeGen();
  Value *r = m_rhs->CodeGen();
  if (!l || !r) return nullptr;

  // Local value names for instructions are purely optional, but it makes it much easier to read the IR dumps
  // LLVM will automatically provide each one with an increasing, unique numeric suffix
  switch (m_op) {
    case '+':
      return builder.CreateFAdd(l, r, "addtmp");
    case '-':
      return builder.CreateFSub(l, r, "subtmp");
    case '*':
      return builder.CreateFMul(l, r, "multmp");
    case '<':
      l = builder.CreateFCmpULT(l, r, "cmptmp");
      // Convert bool 0/1 to double 0.0 or 1.0
      return builder.CreateUIToFP(l, Type::getDoubleTy(the_context), "booltmp");
    default:
      return LogErrorV("invalid binary operator");
  }
}

llvm::Value *CallExprAST::CodeGen() {
  // Lookup name in global mudule table
  // LLVM Module is the container that holds the functions we are JIT’ing
  Function *calleef = the_module->getFunction(m_callee);
  if (!calleef)
    return LogErrorV("Unknown function referenced");

  // If argument mismatch error.
  if (calleef->arg_size() != m_args.size())
    return LogErrorV("Incorrect # arguments passed");

  std::vector<Value*> argsv;
  for (auto & m_arg : m_args) {
    argsv.push_back(m_arg->CodeGen());
    if (!argsv.back())
      return nullptr;
  }
  return builder.CreateCall(calleef, argsv, "calltmp");
}
