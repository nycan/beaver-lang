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
#include <string>
#include <vector>
#include <memory>

class GeneratorData{
public:
    std::unique_ptr<llvm::LLVMContext> m_context;
    std::unique_ptr<llvm::IRBuilder<>> m_builder;
    std::unique_ptr<llvm::Module> m_module;
    std::map<std::string,llvm::Value*> m_namedValues;
};

class SyntaxTree{
public:
    std::unique_ptr<GeneratorData> m_generator;

public:
    SyntaxTree(std::unique_ptr<GeneratorData> t_generator): m_generator(std::move(t_generator)) {}
    virtual ~SyntaxTree() = default;
    virtual llvm::Value* codegen() = 0;
};

class NumberAST : public SyntaxTree{
private:
    double m_value;

public:
    NumberAST(std::unique_ptr<GeneratorData> t_generator, const double t_value):
        SyntaxTree(std::move(t_generator)), m_value(t_value) {}
    llvm::Value* codegen() override {
        return llvm::ConstantFP::get(*(m_generator->m_context),llvm::APFloat(m_value));
    };
};

class VariableAST : public SyntaxTree{
private:
    std::string m_name;

public:
    VariableAST(std::unique_ptr<GeneratorData> t_generator, const std::string& t_name):
        SyntaxTree(std::move(t_generator)), m_name(t_name) {}
    llvm::Value* codegen() override {
        llvm::Value* variable = m_generator->m_namedValues[m_name];
        if (!variable){
            std::cerr << "Unknown variable name.\n";
        }
        return variable;
    };
};

class BinaryOpAST : public SyntaxTree{
private:
    char m_op;
    std::unique_ptr<SyntaxTree> m_lhs, m_rhs;

public:
    BinaryOpAST(std::unique_ptr<GeneratorData> t_generator,
                const char t_op,
                std::unique_ptr<SyntaxTree> t_lhs,
                std::unique_ptr<SyntaxTree> t_rhs
                ): SyntaxTree(std::move(t_generator)), 
                   m_op(t_op), m_lhs(std::move(t_lhs)), m_rhs(std::move(t_rhs)) {}
    llvm::Value* codegen() override {
        llvm::Value* leftCode = m_lhs->codegen();
        llvm::Value* rightCode = m_rhs->codegen();
        if(!leftCode || !rightCode){
            return nullptr;
        }
        switch(m_op){
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
                    llvm::Type::getDoubleTy(*(m_generator->m_context))
                );
            case '>':
                return m_generator->m_builder->CreateUIToFP(
                    m_generator->m_builder->CreateFCmpULT(leftCode,rightCode),
                    llvm::Type::getDoubleTy(*(m_generator->m_context))
                );
            default:
                std::cerr << "Unknown operator\n";
                return nullptr;
        }
    };
};

class CallAST : public SyntaxTree{
private:
    std::string m_callee;
    std::vector<std::unique_ptr<SyntaxTree>> m_args;

public:
    CallAST(std::unique_ptr<GeneratorData> t_generator,
            const std::string& t_callee,
            std::vector<std::unique_ptr<SyntaxTree>> t_args
            ): SyntaxTree(std::move(t_generator)), m_callee(t_callee), m_args(std::move(t_args)) {}
    llvm::Value* codegen() override {
        llvm::Function* calleeCode = m_generator->m_module->getFunction(m_callee);
        if(!calleeCode){
            std::cerr << "Unknown function\n";
            return nullptr;
        }

        std::size_t numArgs = m_args.size();
        if(calleeCode->arg_size() != numArgs){
            std::cerr << "Incorrect number of arguments\n";
            return nullptr;
        }

        std::vector<llvm::Value*> argsCode(numArgs);
        for(unsigned int i = 0; i < numArgs; ++i){
            argsCode[i] = m_args[i]->codegen();
            if(!argsCode[i]){
                return nullptr;
            }
        }

        return m_generator->m_builder->CreateCall(calleeCode, argsCode);
    };
};

class PrototypeAST {
private:
    std::string m_name;
    std::vector<std::string> m_args;

public:
    PrototypeAST(const std::string& t_name, std::vector<std::string> t_args):
                 m_name(t_name), m_args(std::move(t_args)) {}
    const std::string& getName() const {return m_name;}
};

class FunctionAST {
private:
    std::unique_ptr<PrototypeAST> m_prototype;
    std::unique_ptr<SyntaxTree> m_body;

public:
    FunctionAST(std::unique_ptr<PrototypeAST> t_prototype,
                std::unique_ptr<SyntaxTree> t_body
                ): m_prototype(std::move(t_prototype)), m_body(std::move(t_body)) {}
};

#endif