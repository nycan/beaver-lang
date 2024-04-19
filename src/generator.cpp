#include "generator.hpp"


Generator::Generator() {
  m_context = std::make_unique<llvm::LLVMContext>();
  m_module = std::make_unique<llvm::Module>("", *m_context);
  m_builder = std::make_unique<llvm::IRBuilder<>>(*m_context);

  m_funcPass = std::make_unique<llvm::FunctionPassManager>();
  m_loopAnalyzer = std::make_unique<llvm::LoopAnalysisManager>();
  m_funcAnalyzer = std::make_unique<llvm::FunctionAnalysisManager>();
  m_callAnalyzer = std::make_unique<llvm::CGSCCAnalysisManager>();
  m_moduleAnalyzer = std::make_unique<llvm::ModuleAnalysisManager>();
  m_callbacks = std::make_unique<llvm::PassInstrumentationCallbacks>();
  m_instrumentations =
      std::make_unique<llvm::StandardInstrumentations>(*m_context, true);

  m_instrumentations->registerCallbacks(*m_callbacks, m_moduleAnalyzer.get());

  m_funcPass->addPass(llvm::InstCombinePass());
  m_funcPass->addPass(llvm::ReassociatePass());
  m_funcPass->addPass(llvm::GVNPass());
  m_funcPass->addPass(llvm::SimplifyCFGPass());

  llvm::PassBuilder passBuilder;
  passBuilder.registerModuleAnalyses(*m_moduleAnalyzer);
  passBuilder.registerCGSCCAnalyses(*m_callAnalyzer);
  passBuilder.registerFunctionAnalyses(*m_funcAnalyzer);
  passBuilder.registerLoopAnalyses(*m_loopAnalyzer);
  passBuilder.crossRegisterProxies(*m_loopAnalyzer, *m_funcAnalyzer,
                                   *m_callAnalyzer, *m_moduleAnalyzer);

  m_optimizer =
      passBuilder.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2);
}