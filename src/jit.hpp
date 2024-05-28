#ifndef BEAVER_JIT_HPP
#define BEAVER_JIT_HPP
// Note: this file is currently not used

#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LLVMContext.h"
#include <memory>

class BeaverJIT {
private:
  llvm::orc::ExecutionSession m_execSession;
  llvm::orc::RTDyldObjectLinkingLayer m_objLayer;
  llvm::orc::IRCompileLayer m_compLayer;

  llvm::DataLayout m_layout;
  llvm::orc::MangleAndInterner m_mangler;
  llvm::orc::ThreadSafeContext m_context;

public:
  BeaverJIT(llvm::orc::JITTargetMachineBuilder t_jtmb,
              llvm::DataLayout t_layout)
      : m_execSession(std::make_unique<llvm::orc::ExecutorProcessControl>()),
        m_objLayer(
            m_execSession,
            []() { return std::make_unique<llvm::SectionMemoryManager>(); }),
        m_compLayer(m_execSession, m_objLayer,
                    std::make_unique<llvm::orc::ConcurrentIRCompiler>(
                        std::move(t_jtmb))),
        m_layout(std::move(t_layout)), m_mangler(m_execSession, this->m_layout),
        m_context(std::make_unique<llvm::LLVMContext>()) {}
};

#endif // BEAVER_JIT_HPP