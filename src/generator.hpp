#ifndef TESTLANG_GENERATOR_HPP
#define TESTLANG_GENERATOR_HPP

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include <map>
#include <memory>

// Generator class
// Everything related to creating the module is in this class, including optimizations
struct Generator {
  // stuff for generation
  llvm::LLVMContext m_context;
  llvm::IRBuilder<> m_builder;
  llvm::Module m_module;
  std::map<std::string, llvm::Value *> m_namedValues;

  // stuff for optimization
  llvm::FunctionPassManager m_funcPass;
  llvm::LoopAnalysisManager m_loopAnalyzer;
  llvm::FunctionAnalysisManager m_funcAnalyzer;
  llvm::CGSCCAnalysisManager m_callAnalyzer;
  llvm::ModuleAnalysisManager m_moduleAnalyzer;
  llvm::PassInstrumentationCallbacks m_callbacks;
  llvm::StandardInstrumentations m_instrumentations;

  llvm::ModulePassManager m_optimizer;

  Generator();
};

#endif