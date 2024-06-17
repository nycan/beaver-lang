#ifndef BEAVER_OPERATIONS_HPP
#define BEAVER_OPERATIONS_HPP

#include "generator.hpp"
#include "llvm/IR/IRBuilder.h"
#include <map>
#include <optional>
#include <string>

// defines an arbitrary operation
// everything is public since they will all be constant
struct Operation {
  const int precedence;
  llvm::Value *(*codegen)(std::shared_ptr<Generator>, llvm::Value *,
                          llvm::Value *);
};

namespace operations {
// Arithmetic operations
const Operation ADD = {3, [](std::shared_ptr<Generator> t_gen,
                             llvm::Value *t_lhs, llvm::Value *t_rhs) {
                         return t_gen->m_builder.CreateFAdd(t_lhs, t_rhs);
                       }};
const Operation SUB = {3, [](std::shared_ptr<Generator> t_gen,
                             llvm::Value *t_lhs, llvm::Value *t_rhs) {
                         return t_gen->m_builder.CreateFSub(t_lhs, t_rhs);
                       }};
const Operation MULT = {4, [](std::shared_ptr<Generator> t_gen,
                              llvm::Value *t_lhs, llvm::Value *t_rhs) {
                          return t_gen->m_builder.CreateFMul(t_lhs, t_rhs);
                        }};
const Operation DIV = {4, [](std::shared_ptr<Generator> t_gen,
                             llvm::Value *t_lhs, llvm::Value *t_rhs) {
                         return t_gen->m_builder.CreateFDiv(t_lhs, t_rhs);
                       }};
const Operation MOD = {4, [](std::shared_ptr<Generator> t_gen,
                             llvm::Value *t_lhs, llvm::Value *t_rhs) {
                         return t_gen->m_builder.CreateFRem(t_lhs, t_rhs);
                       }};

// Comparison operations
const Operation LESSER = {
    2, [](std::shared_ptr<Generator> t_gen, llvm::Value *t_lhs,
          llvm::Value *t_rhs) {
      return t_gen->m_builder.CreateUIToFP(
          t_gen->m_builder.CreateFCmpULT(t_lhs, t_rhs),
          llvm::Type::getDoubleTy(t_gen->m_builder.getContext()));
    }};
const Operation GREATER = {
    2, [](std::shared_ptr<Generator> t_gen, llvm::Value *t_lhs,
          llvm::Value *t_rhs) {
      return t_gen->m_builder.CreateUIToFP(
          t_gen->m_builder.CreateFCmpUGT(t_lhs, t_rhs),
          llvm::Type::getDoubleTy(t_gen->m_builder.getContext()));
    }};
const Operation LESSEREQ = {
    2, [](std::shared_ptr<Generator> t_gen, llvm::Value *t_lhs,
          llvm::Value *t_rhs) {
      return t_gen->m_builder.CreateUIToFP(
          t_gen->m_builder.CreateFCmpULE(t_lhs, t_rhs),
          llvm::Type::getDoubleTy(t_gen->m_builder.getContext()));
    }};
const Operation GREATEREQ = {
    2, [](std::shared_ptr<Generator> t_gen, llvm::Value *t_lhs,
          llvm::Value *t_rhs) {
      return t_gen->m_builder.CreateUIToFP(
          t_gen->m_builder.CreateFCmpUGE(t_lhs, t_rhs),
          llvm::Type::getDoubleTy(t_gen->m_builder.getContext()));
    }};
const Operation EQUALTO = {
    1, [](std::shared_ptr<Generator> t_gen, llvm::Value *t_lhs,
          llvm::Value *t_rhs) {
      return t_gen->m_builder.CreateUIToFP(
          t_gen->m_builder.CreateFCmpUEQ(t_lhs, t_rhs),
          llvm::Type::getDoubleTy(t_gen->m_builder.getContext()));
    }};
const Operation NOTEQTO = {
    1, [](std::shared_ptr<Generator> t_gen, llvm::Value *t_lhs,
          llvm::Value *t_rhs) {
      return t_gen->m_builder.CreateUIToFP(
          t_gen->m_builder.CreateFCmpUNE(t_lhs, t_rhs),
          llvm::Type::getDoubleTy(t_gen->m_builder.getContext()));
    }};

// Assignment operators
const Operation ASSIGN = {0, [](std::shared_ptr<Generator> t_gen,
                                llvm::Value *t_lhs, llvm::Value *t_rhs) {
                            t_gen->m_builder.CreateStore(t_rhs, t_lhs);
                            return t_rhs;
                          }};

const Operation PLUSEQ = {
    0, [](std::shared_ptr<Generator> t_gen, llvm::Value *t_lhs,
          llvm::Value *t_rhs) {
      llvm::LoadInst *load = t_gen->m_builder.CreateLoad(
          llvm::Type::getDoubleTy(t_gen->m_context), t_lhs);
      llvm::Value *res = t_gen->m_builder.CreateFAdd(load, t_rhs);
      t_gen->m_builder.CreateStore(res, t_lhs);
      return res;
    }};

const Operation MINUSEQ = {
    0, [](std::shared_ptr<Generator> t_gen, llvm::Value *t_lhs,
          llvm::Value *t_rhs) {
      llvm::LoadInst *load = t_gen->m_builder.CreateLoad(
          llvm::Type::getDoubleTy(t_gen->m_context), t_lhs);
      llvm::Value *res = t_gen->m_builder.CreateFSub(load, t_rhs);
      t_gen->m_builder.CreateStore(res, t_lhs);
      return res;
    }};

const Operation TIMESEQ = {
    0, [](std::shared_ptr<Generator> t_gen, llvm::Value *t_lhs,
          llvm::Value *t_rhs) {
      llvm::LoadInst *load = t_gen->m_builder.CreateLoad(
          llvm::Type::getDoubleTy(t_gen->m_context), t_lhs);
      llvm::Value *res = t_gen->m_builder.CreateFMul(load, t_rhs);
      t_gen->m_builder.CreateStore(res, t_lhs);
      return res;
    }};

const Operation DIVEQ = {
    0, [](std::shared_ptr<Generator> t_gen, llvm::Value *t_lhs,
          llvm::Value *t_rhs) {
      llvm::LoadInst *load = t_gen->m_builder.CreateLoad(
          llvm::Type::getDoubleTy(t_gen->m_context), t_lhs);
      llvm::Value *res = t_gen->m_builder.CreateFDiv(load, t_rhs);
      t_gen->m_builder.CreateStore(res, t_lhs);
      return res;
    }};

const Operation MODEQ = {
    0, [](std::shared_ptr<Generator> t_gen, llvm::Value *t_lhs,
          llvm::Value *t_rhs) {
      llvm::LoadInst *load = t_gen->m_builder.CreateLoad(
          llvm::Type::getDoubleTy(t_gen->m_context), t_lhs);
      llvm::Value *res = t_gen->m_builder.CreateFRem(load, t_rhs);
      t_gen->m_builder.CreateStore(res, t_lhs);
      return res;
    }};

// Map of symbols to operations
const std::map<std::string, Operation> opKeys = {
    {"+", ADD},        {"-", SUB},      {"*", MULT},     {"/", DIV},
    {"%", MOD},        {"<", LESSER},   {">", GREATER},  {"<=", LESSEREQ},
    {">=", GREATEREQ}, {"==", EQUALTO}, {"!=", NOTEQTO}};

// These are stored differently because the LHS needs to be a l-value
const std::map<std::string, Operation> assignmentKeys = {{"=", ASSIGN},
                                                         {"+=", PLUSEQ},
                                                         {"-=", MINUSEQ},
                                                         {"*=", TIMESEQ},
                                                         {"/=", DIVEQ},
                                                         {"%=", MODEQ}};

} // namespace operations

// find operation given text
std::optional<Operation> getBinOp(std::string t_key);
std::optional<Operation> getAssignmentOp(std::string t_key);

#endif // BEAVER_OPERATIONS_HPP