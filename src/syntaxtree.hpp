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
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <map>

// class to store all the things needed for code generation & optimization
class Generator{
public:
    // stuff for generation
    std::unique_ptr<llvm::LLVMContext> m_context;
    std::unique_ptr<llvm::IRBuilder<>> m_builder;
    std::unique_ptr<llvm::Module> m_module;
    std::map<std::string,llvm::Value*> m_namedValues;

    // stuff for optimization
    std::unique_ptr<llvm::FunctionPassManager> m_funcPass;
    std::unique_ptr<llvm::LoopAnalysisManager> m_loopAnalyzer;
    std::unique_ptr<llvm::FunctionAnalysisManager> m_funcAnalyzer;
    std::unique_ptr<llvm::CGSCCAnalysisManager> m_callAnalyzer;
    std::unique_ptr<llvm::ModuleAnalysisManager> m_moduleAnalyzer;
    std::unique_ptr<llvm::PassInstrumentationCallbacks> m_callbacks;
    std::unique_ptr<llvm::StandardInstrumentations> m_instrumentations;
    
    llvm::ModulePassManager m_optimizer;

    Generator(){
        m_context = std::make_unique<llvm::LLVMContext>();
        m_module = std::make_unique<llvm::Module>("",*m_context);
        m_builder = std::make_unique<llvm::IRBuilder<>>(*m_context);

        m_funcPass = std::make_unique<llvm::FunctionPassManager>();
        m_loopAnalyzer = std::make_unique<llvm::LoopAnalysisManager>();
        m_funcAnalyzer = std::make_unique<llvm::FunctionAnalysisManager>();
        m_callAnalyzer = std::make_unique<llvm::CGSCCAnalysisManager>();
        m_moduleAnalyzer = std::make_unique<llvm::ModuleAnalysisManager>();
        m_callbacks = std::make_unique<llvm::PassInstrumentationCallbacks>();
        m_instrumentations = std::make_unique<llvm::StandardInstrumentations>(
            *m_context, true
        );
        
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
        passBuilder.crossRegisterProxies(
            *m_loopAnalyzer,*m_funcAnalyzer,*m_callAnalyzer,*m_moduleAnalyzer
        );

        m_optimizer = passBuilder.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2);
    }

    inline void print(){
        m_module->print(llvm::errs(),nullptr);
    }
};

// base AST class
class SyntaxTree{
public:
    std::shared_ptr<Generator> m_generator;

public:
    SyntaxTree(std::shared_ptr<Generator> t_generator): m_generator(t_generator) {}
    virtual ~SyntaxTree() = default;
    virtual llvm::Value* codegen() = 0;
};

class NumberAST : public SyntaxTree{
private:
    double m_value;

public:
    NumberAST(std::shared_ptr<Generator> t_generator, const double t_value):
        SyntaxTree(t_generator), m_value(t_value) {}
    llvm::Value* codegen() override {
        return llvm::ConstantFP::get(*m_generator->m_context,llvm::APFloat(m_value));
    };
};

class VariableAST : public SyntaxTree{
private:
    std::string m_name;

public:
    VariableAST(std::shared_ptr<Generator> t_generator, const std::string& t_name):
        SyntaxTree(t_generator), m_name(t_name) {}
    llvm::Value* codegen() override {
        // search in named variables
        llvm::Value* variable = m_generator->m_namedValues[m_name];
        if (!variable){
            std::cerr << "Unknown variable name.\n";
        }

        return variable;
    };
};

// binary operations
class BinaryOpAST : public SyntaxTree{
private:
    char m_op;
    std::unique_ptr<SyntaxTree> m_lhs, m_rhs;

public:
    BinaryOpAST(std::shared_ptr<Generator> t_generator,
                const char t_op,
                std::unique_ptr<SyntaxTree> t_lhs,
                std::unique_ptr<SyntaxTree> t_rhs
                ): SyntaxTree(t_generator), 
                   m_op(t_op), m_lhs(std::move(t_lhs)), m_rhs(std::move(t_rhs)) {}
    llvm::Value* codegen() override {
        llvm::Value* leftCode = m_lhs->codegen();
        llvm::Value* rightCode = m_rhs->codegen();
        if(!leftCode || !rightCode){
            return nullptr;
        }
        switch(m_op){ // this will probably be replaced at some point...
            case '+':
                return m_generator->m_builder->CreateFAdd(leftCode,rightCode);
            case '-':
                return m_generator->m_builder->CreateFSub(leftCode,rightCode);
            case '*':
                return m_generator->m_builder->CreateFMul(leftCode,rightCode);
            case '/':
                return m_generator->m_builder->CreateFDiv(leftCode,rightCode);
            case '<':
                return m_generator->m_builder->CreateUIToFP(
                    m_generator->m_builder->CreateFCmpULT(leftCode,rightCode),
                    llvm::Type::getDoubleTy(*m_generator->m_context)
                );
            case '>':
                return m_generator->m_builder->CreateUIToFP(
                    m_generator->m_builder->CreateFCmpULT(leftCode,rightCode),
                    llvm::Type::getDoubleTy(*m_generator->m_context)
                );
            default:
                std::cerr << "Unknown operator\n";
                return nullptr;
        }
    };
};

// function calls
class CallAST : public SyntaxTree{
private:
    std::string m_callee;
    std::vector<std::unique_ptr<SyntaxTree>> m_args;

public:
    CallAST(std::shared_ptr<Generator> t_generator,
            const std::string& t_callee,
            std::vector<std::unique_ptr<SyntaxTree>> t_args
            ): SyntaxTree(t_generator), m_callee(t_callee), m_args(std::move(t_args)) {}
    llvm::Value* codegen() override {
        // search for the function being called
        llvm::Function* calledFunction = m_generator->m_module->getFunction(m_callee); 
        if(!calledFunction){
            std::cerr << "Unknown function\n";
            return nullptr;
        }

        // check for number of arguments
        size_t numArgs = m_args.size();
        if(calledFunction->arg_size() != numArgs){
            std::cerr << "Incorrect number of arguments\n";
            return nullptr;
        }

        // generate code for each argument
        std::vector<llvm::Value*> argsCode(numArgs);
        for(size_t i = 0; i < numArgs; ++i){
            argsCode[i] = m_args[i]->codegen();
            if(!argsCode[i]){
                return nullptr;
            }
        }

        return m_generator->m_builder->CreateCall(calledFunction, argsCode);
    };
};

// if/else
class ConditionalAST : public SyntaxTree {
private:
    std::unique_ptr<SyntaxTree> m_condition;
    std::unique_ptr<BlockAST> m_mainBlock, m_elseBlock;
public:
    ConditionalAST(std::shared_ptr<Generator> t_generator,
                   std::unique_ptr<SyntaxTree> t_condition,
                   std::unique_ptr<BlockAST> t_mainBlock,
                   std::unique_ptr<BlockAST> t_elseBlock
                   ): SyntaxTree(t_generator),
                      m_condition(std::move(t_condition)),
                      m_mainBlock(std::move(t_mainBlock)),
                      m_elseBlock(std::move(t_elseBlock)) {}
    ~ConditionalAST() = default;

    llvm::Value* codegen() override {
        //creates main block, else block, and merge block
        //then assigns the correct branching

        // condition
        llvm::Value* conditionalCode = m_condition->codegen();
        if(!conditionalCode){
            return nullptr;
        }
        
        // compare to 0
        conditionalCode = m_generator->m_builder->CreateFCmpONE(
            conditionalCode,
            llvm::ConstantFP::get(*m_generator->m_context,llvm::APFloat(0.0))
        );

        // create blocks
        llvm::Function* functionCode = m_generator->m_builder->GetInsertBlock()->getParent();
        m_mainBlock->m_block = llvm::BasicBlock::Create(*m_generator->m_context,"",functionCode);
        m_elseBlock->m_block = llvm::BasicBlock::Create(*m_generator->m_context);
        llvm::BasicBlock* mergedBB = llvm::BasicBlock::Create(*m_generator->m_context);

        // create the conditional branch
        m_generator->m_builder->CreateCondBr(
            conditionalCode,m_mainBlock->m_block,m_elseBlock->m_block
        );

        llvm::Value* mainCode = m_mainBlock->codegen();
        if(!mainCode){
            return nullptr;
        }

        // after it's finished, go to the merged block
        m_generator->m_builder->CreateBr(mergedBB);

        // create branch and assign to else block
        functionCode->insert(functionCode->end(),m_elseBlock->m_block);

        // generate code for the else block
        // for now, just give "0" for an empty else block
        llvm::Value* elseCode = llvm::ConstantFP::get(
            *m_generator->m_context,llvm::APFloat(0.0)
        );
        if(m_elseBlock){
            elseCode = m_elseBlock->codegen();
            if(!elseCode){
                return nullptr;
            }
        } else {
            m_generator->m_builder->SetInsertPoint(m_elseBlock->m_block);
        }

        //go back to merged blcok
        m_generator->m_builder->CreateBr(mergedBB);

        // in case insert point was changed during code generation
        elseBB = m_generator->m_builder->GetInsertBlock();

        // create merged block
        functionCode->insert(functionCode->end(), mergedBB);
        m_generator->m_builder->SetInsertPoint(mergedBB);
        llvm::PHINode* phiNode = m_generator->m_builder->CreatePHI(
            llvm::Type::getDoubleTy(*m_generator->m_context), 2
        );
        phiNode->addIncoming(mainCode, mainBB);
        phiNode->addIncoming(elseCode, elseBB);
        return phiNode;
    }
};

class PrototypeAST {
private:
    std::shared_ptr<Generator> m_generator;
    std::string m_name;
    std::vector<std::string> m_args;

public:
    PrototypeAST(std::shared_ptr<Generator> t_generator,
                 const std::string& t_name,
                 std::vector<std::string> t_args
                 ): m_generator(t_generator), m_name(t_name), m_args(std::move(t_args)) {}
    ~PrototypeAST() = default;
    const std::string& getName() const {return m_name;}

    llvm::Function* codegen() {
        //all doubles for now
        std::vector<llvm::Type*> tmpType(m_args.size(),llvm::Type::getDoubleTy(*m_generator->m_context));
        
        llvm::FunctionType* funcType = llvm::FunctionType::get(
            llvm::Type::getDoubleTy(*m_generator->m_context),
            tmpType,
            false
        );

        // add the function to the functions table
        llvm::Function* funcCode = llvm::Function::Create(
            funcType, llvm::Function::InternalLinkage, m_name, m_generator->m_module.get()
        );

        // add the argument to the variables table
        size_t it = 0;
        for(auto& arg : funcCode->args()){
            arg.setName(m_args[it++]);
        }

        return funcCode;
    }
};

// return values
class ReturnAST : public SyntaxTree{
private:
    std::unique_ptr<SyntaxTree> m_expression;

public:
    ReturnAST(std::shared_ptr<Generator> t_generator, std::unique_ptr<SyntaxTree> t_expression):
        SyntaxTree(t_generator), m_expression(std::move(t_expression)) {}
    ~ReturnAST() = default;

    llvm::Value* codegen() override {
        if(auto exprCode = m_expression->codegen()){
            return m_generator->m_builder->CreateRet(exprCode);
        }
        return nullptr;
    }
};

// function definitions
class FunctionAST {
private:
    std::shared_ptr<Generator> m_generator;
    std::unique_ptr<PrototypeAST> m_prototype;
    std::unique_ptr<BlockAST> m_body;

public:
    FunctionAST(std::shared_ptr<Generator> t_generator,
                std::unique_ptr<PrototypeAST> t_prototype,
                std::unique_ptr<BlockAST> t_body
                ): m_generator(t_generator), 
                   m_prototype(std::move(t_prototype)), m_body(std::move(t_body)) {}
    ~FunctionAST() = default;

    llvm::Function* codegen(){
        // check for existing function
        llvm::Function* funcCode = m_generator->m_module->getFunction(m_prototype->getName());

        // create if it doesn't exist
        if(!funcCode){
            funcCode = m_prototype->codegen();
        }

        if(!funcCode){
            return nullptr;
        }
        if(!funcCode->empty()){
            std::cerr << "Cannot redefine function.\n";
            return nullptr;
        }

        // parse the body
        m_body->m_block = llvm::BasicBlock::Create( // creates the "block" to be jumped to
            *m_generator->m_context, "", funcCode
        );
        
        // make the only named values the ones defined in the prototype
        m_generator->m_namedValues.clear();
        for(auto& arg : funcCode->args()){
            m_generator->m_namedValues[static_cast<std::string>(arg.getName())] = &arg;
        }

        // parse body
        if(llvm::Value* bodyCode = m_body->codegen()){
            llvm::verifyFunction(*funcCode);

            //run optimizations
            m_generator->m_funcPass->run(*funcCode, *(m_generator->m_funcAnalyzer));
            return funcCode;
        }

        funcCode->eraseFromParent();
        return nullptr;
    }
};

// blocks for conditionals, functions, loops, etc.
class BlockAST {
private:
    std::shared_ptr<Generator> m_generator;
    std::vector<std::unique_ptr<SyntaxTree>> m_lines;

public:
    llvm::BasicBlock* m_block;

    BlockAST(std::shared_ptr<Generator> t_generator, std::vector<std::unique_ptr<SyntaxTree>> t_lines):
        m_generator(t_generator), m_lines(std::move(t_lines)) {}
    ~BlockAST() = default;

    llvm::Value* codegen() {
        m_generator->m_builder->SetInsertPoint(m_block);

        for(auto& line : m_lines){
            if(!line->codegen()){
                return nullptr;
            }
        }

        return m_block = m_generator->m_builder->GetInsertBlock();
    }
};

#endif // TESTLANG_SYNTAXTREE_HPP