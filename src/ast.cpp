//
// Created by zhong on 7/4/21.
//

#include "ast.h"
#include "logger.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

using namespace llvm;
using namespace llvm::orc;

// TheContext is an opaque object that owns a lot of core LLVM data structures.
std::unique_ptr<LLVMContext> the_context;
// The Builder object is a helper object that makes it easy to generate LLVM instructions.
static std::unique_ptr<IRBuilder<>> builder;
// TheModule is an LLVM construct that contains functions and global variables.
std::unique_ptr<Module> the_module;
// Symbol table for the code.
std::map<std::string, Value *> named_values;

std::unique_ptr<legacy::FunctionPassManager> the_fpm;

std::unique_ptr<KaleidoscopeJIT> the_jit;

std::map<std::string, std::unique_ptr<PrototypeAST>> function_protos;

void InitializeModuleAndPassManager() {
  the_context = std::make_unique<LLVMContext>();
  the_module = std::make_unique<Module>("my cool jit", *the_context);
  the_module->setDataLayout(the_jit->getDataLayout());

  // Create a new builder for the module.
  builder = std::make_unique<IRBuilder<>>(*the_context);

  // Create a new pass manager
  the_fpm = std::make_unique<legacy::FunctionPassManager>(the_module.get());

  // Do simple "peephole" optimizations and bit-twiddling optzns.
  the_fpm->add(createInstructionCombiningPass());
  // Reassociate expressions.
  the_fpm->add(createReassociatePass());
  // Eliminate Common SubExpressions.
  the_fpm->add(createGVNPass());
  // Simplify the control flow graph (deleting unreachable blocks, etc).
  the_fpm->add(createCFGSimplificationPass());

  the_fpm->doInitialization();
}

llvm::Value *NumberExprAST::CodeGen() {
  return ConstantFP::get(*the_context, APFloat(m_val));
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
    case '+':return builder->CreateFAdd(l, r, "addtmp");
    case '-':return builder->CreateFSub(l, r, "subtmp");
    case '*':return builder->CreateFMul(l, r, "multmp");
    case '<':l = builder->CreateFCmpULT(l, r, "cmptmp");
      // Convert bool 0/1 to double 0.0 or 1.0
      return builder->CreateUIToFP(l, Type::getDoubleTy(*the_context), "booltmp");
    default:return LogErrorV("invalid binary operator");
  }
}

llvm::Value *CallExprAST::CodeGen() {
  // Lookup name in global mudule table
  // LLVM Module is the container that holds the functions we are JIT???ing
  Function *calleef = the_module->getFunction(m_callee);
  if (!calleef)
    return LogErrorV("Unknown function referenced");

  // If argument mismatch error.
  if (calleef->arg_size() != m_args.size())
    return LogErrorV("Incorrect # arguments passed");

  std::vector<Value *> argsv;
  for (auto &m_arg : m_args) {
    argsv.push_back(m_arg->CodeGen());
    if (!argsv.back())
      return nullptr;
  }
  return builder->CreateCall(calleef, argsv, "calltmp");
}

llvm::Function *PrototypeAST::CodeGen() {
  std::vector<Type *> doubles(m_ars.size(), Type::getDoubleTy(*the_context));
  FunctionType *ft = FunctionType::get(Type::getDoubleTy(*the_context), doubles, false);
  Function *f = Function::Create(ft, Function::ExternalLinkage, m_name, the_module.get());
  // Not strictly necessary, bug keeping the name to make IR more readable.
  uint32_t idx = 0;
  for (auto &arg : f->args())
    arg.setName(m_ars[idx++]);
  return f;
}

llvm::Function *FunctionAST::CodeGen() {
  Function *the_function = the_module->getFunction(m_proto->GetName());
  if (!the_function)
    the_function = m_proto->CodeGen();
  // Has no body yet
  if (!the_function->empty())
    return (Function *) LogErrorV("Function cannot be redefined.");

  BasicBlock *bb = BasicBlock::Create(*the_context, "entry", the_function);
  builder->SetInsertPoint(bb);

  // Record the function arguments
  named_values.clear();
  for (auto &arg : the_function->args())
    named_values[arg.getName().str()] = &arg;

  if (Value *retval = m_body->CodeGen()) {
    // Finish off the function
    builder->CreateRet(retval);
    // Validate the generated code
    verifyFunction(*the_function);

    // Optimize the function
    the_fpm->run(*the_function);
    return the_function;
  }

  // Error reading body, remove function
  the_function->eraseFromParent();
  return nullptr;
}
