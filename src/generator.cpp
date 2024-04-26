#include "generator.hpp"

Generator::Generator()
    : m_context(), m_module("", m_context), m_builder(m_context),
      m_instrumentations(m_context, true) {

  m_instrumentations.registerCallbacks(m_callbacks, &m_moduleAnalyzer);

  m_funcPass.addPass(llvm::InstCombinePass());
  m_funcPass.addPass(llvm::ReassociatePass());
  m_funcPass.addPass(llvm::GVNPass());
  m_funcPass.addPass(llvm::SimplifyCFGPass());

  llvm::PassBuilder passBuilder;
  passBuilder.registerModuleAnalyses(m_moduleAnalyzer);
  passBuilder.registerCGSCCAnalyses(m_callAnalyzer);
  passBuilder.registerFunctionAnalyses(m_funcAnalyzer);
  passBuilder.registerLoopAnalyses(m_loopAnalyzer);
  passBuilder.crossRegisterProxies(m_loopAnalyzer, m_funcAnalyzer,
                                   m_callAnalyzer, m_moduleAnalyzer);

  m_optimizer =
      passBuilder.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2);
}