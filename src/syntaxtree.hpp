#ifndef BEAVER_SYNTAXTREE_HPP
#define BEAVER_SYNTAXTREE_HPP

#include "generator.hpp"
#include "operations.hpp"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

// Options for result of code generation
enum class GenStatus { ok, terminated, error };

// base AST class
class SyntaxTree {
protected:
  std::shared_ptr<Generator> m_generator;

public:
  SyntaxTree(std::shared_ptr<Generator> t_generator)
      : m_generator(t_generator) {}
  virtual ~SyntaxTree() = default;
  virtual GenStatus codegen() = 0;
  virtual bool terminatesBlock() { return false; }
};

class ExpressionTree : public SyntaxTree {
public:
  ExpressionTree(std::shared_ptr<Generator> t_generator)
      : SyntaxTree(t_generator) {}
  virtual ~ExpressionTree() = default;
  virtual std::optional<llvm::Value *> codegenE() = 0;
  // temporary
  inline GenStatus codegen() override final {
    if (codegenE()) {
      return GenStatus::ok;
    }
    return GenStatus::error;
  }
};

using linePtr = std::unique_ptr<SyntaxTree>;
using expressionPtr = std::unique_ptr<ExpressionTree>;
using blockPtr = std::vector<std::unique_ptr<SyntaxTree>>;

class NumberAST : public ExpressionTree {
private:
  double m_value;

public:
  NumberAST(std::shared_ptr<Generator> t_generator, const double t_value)
      : ExpressionTree(t_generator), m_value(t_value) {}
  std::optional<llvm::Value *> codegenE() override;
};

class VariableAST : public ExpressionTree {
private:
  std::string m_name;

public:
  VariableAST(std::shared_ptr<Generator> t_generator, const std::string &t_name)
      : ExpressionTree(t_generator), m_name(t_name) {}
  std::optional<llvm::Value *> codegenE() override;
};

// binary operations
class BinaryOpAST : public ExpressionTree {
private:
  const Operation m_op;
  expressionPtr m_lhs, m_rhs;

public:
  BinaryOpAST(std::shared_ptr<Generator> t_generator, const Operation t_op,
              expressionPtr t_lhs, expressionPtr t_rhs)
      : ExpressionTree(t_generator), m_op(t_op), m_lhs(std::move(t_lhs)),
        m_rhs(std::move(t_rhs)) {}
  std::optional<llvm::Value *> codegenE() override;
};

// function calls
class CallAST : public ExpressionTree {
private:
  std::string m_callee;
  std::vector<expressionPtr> m_args;

public:
  CallAST(std::shared_ptr<Generator> t_generator, const std::string &t_callee,
          std::vector<expressionPtr> t_args)
      : ExpressionTree(t_generator), m_callee(t_callee),
        m_args(std::move(t_args)) {}
  std::optional<llvm::Value *> codegenE() override;
};

// if/else
class ConditionalAST : public SyntaxTree {
private:
  std::vector<expressionPtr> m_conditions;
  std::vector<blockPtr> m_mainBlocks;
  std::optional<blockPtr> m_elseBlock;

public:
  ConditionalAST(std::shared_ptr<Generator> t_generator,
                 std::vector<expressionPtr> t_conditions,
                 std::vector<blockPtr> t_mainBlocks,
                 std::optional<blockPtr> t_elseBlock)
      : SyntaxTree(t_generator), m_conditions(std::move(t_conditions)),
        m_mainBlocks(std::move(t_mainBlocks)),
        m_elseBlock(std::move(t_elseBlock)) {}
  ~ConditionalAST() = default;

  GenStatus codegen() override;
};

class WhileAST : public SyntaxTree {
private:
  expressionPtr m_condition;
  blockPtr m_block;

public:
  WhileAST(std::shared_ptr<Generator> t_generator, expressionPtr t_condition,
           blockPtr t_block)
      : SyntaxTree(t_generator), m_condition(std::move(t_condition)),
        m_block(std::move(t_block)) {}
  ~WhileAST() = default;

  GenStatus codegen() override;
};

class ForAST : public SyntaxTree {
private:
  linePtr m_initialization;
  expressionPtr m_condition;
  linePtr m_updation;
  blockPtr m_block;

public:
  ForAST(std::shared_ptr<Generator> t_generator, linePtr t_initialization,
         expressionPtr t_condition, linePtr t_updation, blockPtr t_block)
      : SyntaxTree(t_generator), m_initialization(std::move(t_initialization)),
        m_condition(std::move(t_condition)), m_updation(std::move(t_updation)),
        m_block(std::move(t_block)) {}
  ~ForAST() = default;

  GenStatus codegen() override;
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

  std::optional<llvm::Function *> codegen();
};

// return values
class ReturnAST : public SyntaxTree {
private:
  expressionPtr m_expression;

public:
  ReturnAST(std::shared_ptr<Generator> t_generator, expressionPtr t_expression)
      : SyntaxTree(t_generator), m_expression(std::move(t_expression)) {}
  ~ReturnAST() = default;

  GenStatus codegen() override;
  virtual bool terminatesBlock() override { return true; }
};

// function definitions
class FunctionAST {
private:
  std::shared_ptr<Generator> m_generator;
  std::unique_ptr<PrototypeAST> m_prototype;
  blockPtr m_body;

public:
  FunctionAST(std::shared_ptr<Generator> t_generator,
              std::unique_ptr<PrototypeAST> t_prototype, blockPtr t_body)
      : m_generator(t_generator), m_prototype(std::move(t_prototype)),
        m_body(std::move(t_body)) {}
  ~FunctionAST() = default;

  std::optional<llvm::Function *> codegen();
};

#endif // BEAVER_SYNTAXTREE_HPP