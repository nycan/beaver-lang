#include "syntaxtree.hpp"

std::optional<llvm::Value *> NumberAST::codegenE() {
  return llvm::ConstantFP::get(m_generator->m_context, llvm::APFloat(m_value));
};

std::optional<llvm::Value *> VariableAST::codegenE() {
  // search in named variables
  llvm::AllocaInst *variable = m_generator->m_namedValues[m_name];
  if (!variable) {
    llvm::errs() << "Unknown variable name.\n";
    return {};
  }

  return m_generator->m_builder.CreateLoad(variable->getAllocatedType(), variable);
};

std::optional<llvm::Value *> BinaryOpAST::codegenE() {
  std::optional<llvm::Value *> leftCode = m_lhs->codegenE();
  std::optional<llvm::Value *> rightCode = m_rhs->codegenE();
  if (!leftCode || !rightCode) {
    return {};
  }
  return m_op.codegen(m_generator, *leftCode, *rightCode);
};

std::optional<llvm::Value *> CallAST::codegenE() {
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
    std::optional<llvm::Value *> line = m_args[i]->codegenE();
    if (!line) {
      return {};
    }
    argsCode[i] = *line;
  }

  return m_generator->m_builder.CreateCall(calledFunction, argsCode);
};

GenStatus ConditionalAST::codegen() {
  // create blocks
  llvm::Function *functionCode =
      m_generator->m_builder.GetInsertBlock()->getParent();

  llvm::BasicBlock *mergedBB =
      llvm::BasicBlock::Create(m_generator->m_context, "", functionCode);
  llvm::BasicBlock *checkBB = m_generator->m_builder.GetInsertBlock();
  llvm::BasicBlock *nextBB =
      llvm::BasicBlock::Create(m_generator->m_context, "", functionCode);

  unsigned numBlocks = m_conditions.size();
  bool allTerminated = true;

  for (unsigned i = 0; i < numBlocks; ++i) {
    // condition
    std::optional<llvm::Value *> conditionCode = m_conditions[i]->codegenE();
    if (!conditionCode) {
      return GenStatus::error;
    }

    // compare to 0
    llvm::Value *comparisonCode = m_generator->m_builder.CreateFCmpONE(
        *conditionCode,
        llvm::ConstantFP::get(m_generator->m_context, llvm::APFloat(0.0)));

    // create the block with the code
    llvm::BasicBlock *codeBB =
        llvm::BasicBlock::Create(m_generator->m_context, "", functionCode);

    // create the conditional branch
    m_generator->m_builder.CreateCondBr(comparisonCode, codeBB, nextBB);
    m_generator->m_builder.SetInsertPoint(codeBB);

    bool currTerminated = false;
    for (auto &line : m_mainBlocks[i]) {
      GenStatus mainResult = line->codegen();
      if (mainResult == GenStatus::error) {
        return GenStatus::error;
      }
      if (mainResult == GenStatus::terminated) { // only one terminator allowed
        currTerminated = true;
        break;
      }
    }

    // after it's finished, go to the merged block
    if (!currTerminated) {
      allTerminated = false;
      m_generator->m_builder.CreateBr(mergedBB);
    }

    checkBB = nextBB;

    // I feel like there's a better way to do this...
    if (i < numBlocks - 1) {
      nextBB =
          llvm::BasicBlock::Create(m_generator->m_context, "", functionCode);
    }

    m_generator->m_builder.SetInsertPoint(checkBB);
  }

  // checkBB is now the else block

  // generate code for the else block
  bool elseTerminated = false;
  if (m_elseBlock) {
    for (auto &line : *m_elseBlock) {
      GenStatus elseResult = line->codegen();
      if (elseResult == GenStatus::error) {
        return GenStatus::error;
      } else if (elseResult == GenStatus::terminated) {
        elseTerminated = true;
        break;
      }
    }
  }

  // go back to merged block
  if (!elseTerminated) {
    m_generator->m_builder.CreateBr(mergedBB);
  }

  // create merged block
  if (!allTerminated || !elseTerminated) {
    m_generator->m_builder.SetInsertPoint(mergedBB);
  } else {
    llvm::DeleteDeadBlock(mergedBB);
    return GenStatus::terminated;
  }
  // placeholder. the structure will be changed soon
  return GenStatus::ok;
}

GenStatus WhileAST::codegen() {
  // condition
  std::optional<llvm::Value *> conditionCode = m_condition->codegenE();
  if (!conditionCode) {
    return {};
  }

  // compare to 0
  llvm::Value *comparisonCode = m_generator->m_builder.CreateFCmpONE(
      *conditionCode,
      llvm::ConstantFP::get(m_generator->m_context, llvm::APFloat(0.0)));

  // create blocks
  llvm::Function *functionCode =
      m_generator->m_builder.GetInsertBlock()->getParent();
  llvm::BasicBlock *blockBB =
      llvm::BasicBlock::Create(m_generator->m_context, "", functionCode);
  llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(m_generator->m_context);

  // create the conditional branch
  m_generator->m_builder.CreateCondBr(comparisonCode, blockBB, afterBB);

  m_generator->m_builder.SetInsertPoint(blockBB);

  bool terminated = false;
  for (auto &line : m_block) {
    GenStatus lineResult = line->codegen();
    if (lineResult != GenStatus::ok) {
      return lineResult;
    }
  }

  m_generator->m_builder.CreateCondBr(comparisonCode, blockBB, afterBB);
  m_generator->m_builder.SetInsertPoint(afterBB);

  return GenStatus::ok;
}

GenStatus ForAST::codegen() {
  // intialization
  GenStatus initializationResult = m_initialization->codegen();
  if (initializationResult != GenStatus::ok) {
    return initializationResult;
  }

  // condition
  std::optional<llvm::Value *> conditionCode = m_condition->codegenE();
  if (!conditionCode) {
    return {};
  }

  // compare to 0
  llvm::Value *comparisonCode = m_generator->m_builder.CreateFCmpONE(
      *conditionCode,
      llvm::ConstantFP::get(m_generator->m_context, llvm::APFloat(0.0)));

  // create blocks
  llvm::Function *functionCode =
      m_generator->m_builder.GetInsertBlock()->getParent();
  llvm::BasicBlock *blockBB =
      llvm::BasicBlock::Create(m_generator->m_context, "", functionCode);
  llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(m_generator->m_context);

  // create the conditional branch
  m_generator->m_builder.CreateCondBr(comparisonCode, blockBB, afterBB);

  m_generator->m_builder.SetInsertPoint(blockBB);

  bool terminated = false;
  for (auto &line : m_block) {
    GenStatus lineResult = line->codegen();
    if (lineResult != GenStatus::ok) {
      return lineResult;
    }
  }

  // updation
  GenStatus updationResult = m_updation->codegen();
  if (updationResult == GenStatus::error) {
    return GenStatus::error;
  }

  if (!m_updation->terminatesBlock()) {
    m_generator->m_builder.CreateCondBr(comparisonCode, blockBB, afterBB);
  }

  // Todo: just terminate processing if the block is terminated
  m_generator->m_builder.SetInsertPoint(afterBB);

  return GenStatus::ok;
}

GenStatus DeclarationAST::codegen() {
  if (m_generator->m_namedValues.find(m_name) != m_generator->m_namedValues.end()) {
    llvm::errs() << "Variable '" << m_name << "' already exists in this scope.\n";
    return GenStatus::error;
  }
  llvm::AllocaInst *inst = m_generator->m_builder.CreateAlloca(
    llvm::Type::getDoubleTy(m_generator->m_context)
  );
  m_generator->m_namedValues[m_name] = inst;
  
  return GenStatus::ok;
}

std::optional<llvm::Function *> PrototypeAST::codegen() {
  // all doubles for now
  std::vector<llvm::Type *> tmpType(
      m_args.size(), llvm::Type::getDoubleTy(m_generator->m_context));

  llvm::FunctionType *funcType = llvm::FunctionType::get(
      llvm::Type::getDoubleTy(m_generator->m_context), tmpType, false);

  // add the function to the functions table
  llvm::Function *funcCode = llvm::Function::Create(
      funcType, llvm::Function::InternalLinkage, m_name, m_generator->m_module);

  // add the argument to the variables table
  size_t it = 0;
  for (auto &arg : funcCode->args()) {
    arg.setName(m_args[it++]);
  }

  return funcCode;
}

GenStatus ReturnAST::codegen() {
  if (auto exprCode = m_expression->codegenE()) {
    m_generator->m_builder.CreateRet(*exprCode);
    return GenStatus::terminated;
  }
  return GenStatus::error;
}

std::optional<llvm::Function *> FunctionAST::codegen() {
  // check for existing function
  std::optional<llvm::Function *> funcCode =
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
  llvm::BasicBlock *definitionBlock = llvm::BasicBlock::Create(
      m_generator->m_context, "",
      *funcCode); // creates the "block" to be jumped to

  // set code insertion point
  m_generator->m_builder.SetInsertPoint(definitionBlock);

  // make the only named values the ones defined in the prototype
  m_generator->m_namedValues.clear();
  for (auto &arg : (*funcCode)->args()) {
    llvm::AllocaInst *argInst = m_generator->m_builder.CreateAlloca(
      llvm::Type::getDoubleTy(m_generator->m_context)
    );
    m_generator->m_builder.CreateStore(&arg, argInst);
    m_generator->m_namedValues[static_cast<std::string>(arg.getName())] = argInst;
  }

  // parse body
  for (auto &line : m_body) {
    GenStatus lineResult = line->codegen();
    if (lineResult == GenStatus::error) {
      (*funcCode)->eraseFromParent();
      return {};
    } else if (lineResult == GenStatus::terminated) {
      break;
    }
  }

  // verify the generated code
  if (llvm::verifyFunction(**funcCode, &llvm::errs())) {
    return {};
  }

  (*funcCode)->setCallingConv(llvm::CallingConv::C);

  // run optimizations
  m_generator->m_funcPass.run(**funcCode, m_generator->m_funcAnalyzer);

  return funcCode;
}