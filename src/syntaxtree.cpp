#include "syntaxtree.hpp"

std::optional<llvm::Value*> NumberAST::codegen() {
  return llvm::ConstantFP::get(m_generator->m_context, llvm::APFloat(m_value));
};

std::optional<llvm::Value*> VariableAST::codegen() {
  // search in named variables
  llvm::Value *variable = m_generator->m_namedValues[m_name];
  if (!variable) {
    std::cerr << "Unknown variable name.\n";
  }

  return variable;
};

std::optional<llvm::Value*> BinaryOpAST::codegen() {
  std::optional<llvm::Value*> leftCode = m_lhs->codegen();
  std::optional<llvm::Value*> rightCode = m_rhs->codegen();
  if (!leftCode || !rightCode) {
    return {};
  }
  return m_op.codegen(m_generator,*leftCode,*rightCode);
};

std::optional<llvm::Value*> CallAST::codegen() {
  // search for the function being called
  llvm::Function *calledFunction = m_generator->m_module.getFunction(m_callee);
  if (!calledFunction) {
    std::cerr << "Unknown function\n";
    return {};
  }

  // check for number of arguments
  size_t numArgs = m_args.size();
  if (calledFunction->arg_size() != numArgs) {
    std::cerr << "Incorrect number of arguments\n";
    return {};
  }

  // generate code for each argument
  std::vector<llvm::Value *> argsCode(numArgs);
  for (size_t i = 0; i < numArgs; ++i) {
    std::optional<llvm::Value*> line = m_args[i]->codegen();
    if (!line) {
      return {};
    }
    argsCode[i] = *line;
  }

  return m_generator->m_builder.CreateCall(calledFunction, argsCode);
};

std::optional<llvm::Value*> ConditionalAST::codegen() {
  // creates main block, else block, and merge block
  // then assigns the correct branching

  // condition
  std::optional<llvm::Value*> conditionCode = m_condition->codegen();
  if (!conditionCode) {
    return {};
  }

  // compare to 0
  llvm::Value* comparisonCode = m_generator->m_builder.CreateFCmpONE(
      *conditionCode,
      llvm::ConstantFP::get(m_generator->m_context, llvm::APFloat(0.0)));

  // create blocks
  llvm::Function *functionCode =
      m_generator->m_builder.GetInsertBlock()->getParent();
  llvm::BasicBlock *mainBB =
      llvm::BasicBlock::Create(m_generator->m_context, "", functionCode);
  llvm::BasicBlock *elseBB = llvm::BasicBlock::Create(m_generator->m_context);
  llvm::BasicBlock *mergedBB =
      llvm::BasicBlock::Create(m_generator->m_context);

  // create the conditional branch
  m_generator->m_builder.CreateCondBr(comparisonCode, mainBB, elseBB);

  m_generator->m_builder.SetInsertPoint(mainBB);

  bool mainTerminated = false;
  for (auto &line : m_mainBlock) {
    std::optional<llvm::Value*> mainCode = line->codegen();
    if (!mainCode) {
      return {};
    }

    // if block is returns, then don't worry processing the rest
    if (line->terminatesBlock()) {
      mainTerminated = true;
      break;
    }
  }

  // after it's finished, go to the merged block
  if (!mainTerminated) {
    m_generator->m_builder.CreateBr(mergedBB);
  }
  mainBB = m_generator->m_builder.GetInsertBlock();

  // create branch and assign to else block
  functionCode->insert(functionCode->end(), elseBB);
  m_generator->m_builder.SetInsertPoint(elseBB);

  // generate code for the else block
  bool elseTerminated = false;
  if(m_elseBlock){
    for (auto &line : *m_elseBlock) {
      std::optional<llvm::Value*> elseCode = line->codegen();
      if (!elseCode) {
        return {};
      }

      if (line->terminatesBlock()) {
        elseTerminated = true;
        break;
      }
    }
  }

  // go back to merged blcok
  if (!elseTerminated) {
    m_generator->m_builder.CreateBr(mergedBB);
  }

  // in case insert point was changed during code generation
  elseBB = m_generator->m_builder.GetInsertBlock();
  // create merged block
  if (!elseTerminated || !mainTerminated) {
    functionCode->insert(functionCode->end(), mergedBB);
    m_generator->m_builder.SetInsertPoint(mergedBB);
  }
  // llvm::PHINode *phiNode = m_generator->m_builder.CreatePHI(
  //     llvm::Type::getDoubleTy(m_generator->m_context), 2);
  // phiNode->addIncoming(mainCode, mainBB);
  // phiNode->addIncoming(elseCode, elseBB);
  // return phiNode;

  // placeholder. the structure will be changed in next PR
  return mergedBB;
}

std::optional<llvm::Function*> PrototypeAST::codegen() {
  // all doubles for now
  std::vector<llvm::Type *> tmpType(
      m_args.size(), llvm::Type::getDoubleTy(m_generator->m_context));

  llvm::FunctionType *funcType = llvm::FunctionType::get(
      llvm::Type::getDoubleTy(m_generator->m_context), tmpType, false);

  // add the function to the functions table
  llvm::Function *funcCode =
      llvm::Function::Create(funcType, llvm::Function::InternalLinkage, m_name,
                             m_generator->m_module);

  // add the argument to the variables table
  size_t it = 0;
  for (auto &arg : funcCode->args()) {
    arg.setName(m_args[it++]);
  }

  return funcCode;
}

std::optional<llvm::Value*> ReturnAST::codegen() {
  if (auto exprCode = m_expression->codegen()) {
    return m_generator->m_builder.CreateRet(*exprCode);
  }
  return {};
}

std::optional<llvm::Function*> FunctionAST::codegen() {
  // check for existing function
  std::optional<llvm::Function*> funcCode =
      m_generator->m_module.getFunction(m_prototype->getName());

  // create if it doesn't exist
  if (!*funcCode) {
    funcCode = m_prototype->codegen();
  }

  if (!funcCode) {
    return {};
  }
  if (!(*funcCode)->empty()) {
    std::cerr << "Cannot redefine function.\n";
    return {};
  }

  // parse the body
  llvm::BasicBlock *definitionBlock =
      llvm::BasicBlock::Create(m_generator->m_context, "",
                               *funcCode); // creates the "block" to be jumped to

  // set code insertion point
  m_generator->m_builder.SetInsertPoint(definitionBlock);

  // make the only named values the ones defined in the prototype
  m_generator->m_namedValues.clear();
  for (auto &arg : (*funcCode)->args()) {
    m_generator->m_namedValues[static_cast<std::string>(arg.getName())] = &arg;
  }

  // parse body
  for (auto &line : m_body) {
    if (!line->codegen()) {
      (*funcCode)->eraseFromParent();
      return {};
    }
  }

  // verify the generated code
  if (llvm::verifyFunction(**funcCode)) {
    llvm::errs() << "Errors found in code generation.\n";
    return {};
  }

  // run optimizations
  m_generator->m_funcPass.run(**funcCode, m_generator->m_funcAnalyzer);
  return funcCode;
}