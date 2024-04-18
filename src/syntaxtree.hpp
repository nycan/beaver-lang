#ifndef TESTLANG_SYNTAXTREE_HPP
#define TESTLANG_SYNTAXTREE_HPP

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
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "operations.hpp"
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

// class to store all the things needed for code generation & optimization
class Generator {
public:
  // stuff for generation
  std::unique_ptr<llvm::LLVMContext> m_context;
  std::unique_ptr<llvm::IRBuilder<>> m_builder;
  std::unique_ptr<llvm::Module> m_module;
  std::map<std::string, llvm::Value *> m_namedValues;

  // stuff for optimization
  std::unique_ptr<llvm::FunctionPassManager> m_funcPass;
  std::unique_ptr<llvm::LoopAnalysisManager> m_loopAnalyzer;
  std::unique_ptr<llvm::FunctionAnalysisManager> m_funcAnalyzer;
  std::unique_ptr<llvm::CGSCCAnalysisManager> m_callAnalyzer;
  std::unique_ptr<llvm::ModuleAnalysisManager> m_moduleAnalyzer;
  std::unique_ptr<llvm::PassInstrumentationCallbacks> m_callbacks;
  std::unique_ptr<llvm::StandardInstrumentations> m_instrumentations;

  llvm::ModulePassManager m_optimizer;

  inline void print() { m_module->print(llvm::errs(), nullptr); }

  Generator();
};

// base AST class
class SyntaxTree {
public:
  std::shared_ptr<Generator> m_generator;

public:
  SyntaxTree(std::shared_ptr<Generator> t_generator)
      : m_generator(t_generator) {}
  virtual ~SyntaxTree() = default;
  virtual llvm::Value *codegen() = 0;
  virtual bool terminatesBlock() { return false; }
};

class NumberAST : public SyntaxTree {
private:
  double m_value;

public:
  NumberAST(std::shared_ptr<Generator> t_generator, const double t_value)
      : SyntaxTree(t_generator), m_value(t_value) {}
  llvm::Value *codegen() override;
};

class VariableAST : public SyntaxTree {
private:
  std::string m_name;

public:
  VariableAST(std::shared_ptr<Generator> t_generator, const std::string &t_name)
      : SyntaxTree(t_generator), m_name(t_name) {}
  llvm::Value *codegen() override;
};

// binary operations
class BinaryOpAST : public SyntaxTree {
private:
  const Operation m_op;
  std::unique_ptr<SyntaxTree> m_lhs, m_rhs;

public:
  BinaryOpAST(std::shared_ptr<Generator> t_generator, const Operation t_op,
              std::unique_ptr<SyntaxTree> t_lhs,
              std::unique_ptr<SyntaxTree> t_rhs)
      : SyntaxTree(t_generator), m_op(t_op), m_lhs(std::move(t_lhs)),
        m_rhs(std::move(t_rhs)) {}
  llvm::Value *codegen() override;
};

// function calls
class CallAST : public SyntaxTree {
private:
  std::string m_callee;
  std::vector<std::unique_ptr<SyntaxTree>> m_args;

public:
  CallAST(std::shared_ptr<Generator> t_generator, const std::string &t_callee,
          std::vector<std::unique_ptr<SyntaxTree>> t_args)
      : SyntaxTree(t_generator), m_callee(t_callee), m_args(std::move(t_args)) {
  }
  llvm::Value *codegen() override;
};

// if/else
class ConditionalAST : public SyntaxTree {
private:
  std::unique_ptr<SyntaxTree> m_condition;
  std::vector<std::unique_ptr<SyntaxTree>> m_mainBlock, m_elseBlock;

public:
  ConditionalAST(std::shared_ptr<Generator> t_generator,
                 std::unique_ptr<SyntaxTree> t_condition,
                 std::vector<std::unique_ptr<SyntaxTree>> t_mainBlock,
                 std::vector<std::unique_ptr<SyntaxTree>> t_elseBlock)
      : SyntaxTree(t_generator), m_condition(std::move(t_condition)),
        m_mainBlock(std::move(t_mainBlock)),
        m_elseBlock(std::move(t_elseBlock)) {}
  ~ConditionalAST() = default;

  llvm::Value *codegen() override;
};

class PrototypeAST {
private:
  std::shared_ptr<Generator> m_generator;
  std::string m_name;
  std::vector<std::string> m_args;

public:
  PrototypeAST(std::shared_ptr<Generator> t_generator,
               const std::string &t_name, std::vector<std::string> t_args)
      : m_generator(t_generator), m_name(t_name), m_args(std::move(t_args)) {}
  ~PrototypeAST() = default;
  const std::string &getName() const { return m_name; }

  llvm::Function *codegen();
};

// return values
class ReturnAST : public SyntaxTree {
private:
  std::unique_ptr<SyntaxTree> m_expression;

public:
  ReturnAST(std::shared_ptr<Generator> t_generator,
            std::unique_ptr<SyntaxTree> t_expression)
      : SyntaxTree(t_generator), m_expression(std::move(t_expression)) {}
  ~ReturnAST() = default;

  llvm::Value *codegen() override;
  virtual bool terminatesBlock() override { return true; }
};

// function definitions
class FunctionAST {
private:
  std::shared_ptr<Generator> m_generator;
  std::unique_ptr<PrototypeAST> m_prototype;
  std::vector<std::unique_ptr<SyntaxTree>> m_body;

public:
  FunctionAST(std::shared_ptr<Generator> t_generator,
              std::unique_ptr<PrototypeAST> t_prototype,
              std::vector<std::unique_ptr<SyntaxTree>> t_body)
      : m_generator(t_generator), m_prototype(std::move(t_prototype)),
        m_body(std::move(t_body)) {}
  ~FunctionAST() = default;

  llvm::Function *codegen();
};

#endif // TESTLANG_SYNTAXTREE_HPP