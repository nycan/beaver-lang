#ifndef TESTLANG_OPERATIONS_HPP
#define TESTLANG_OPERATIONS_HPP

#include "llvm/IR/IRBuilder.h"
#include <string>

// defines an arbitrary operation
// everything is public since they will all be constant
struct Operation {
    const int precedence;
    llvm::Value* (*codegen)(llvm::IRBuilder<>*, llvm::Value*, llvm::Value*);
};

namespace operations{
    const Operation ADD = {
        2,
        [](llvm::IRBuilder<>* t_builder, llvm::Value* t_lhs, llvm::Value* t_rhs){
            return t_builder->CreateFAdd(t_lhs,t_rhs);
        }
    };
    const Operation SUB = {
        2,
        [](llvm::IRBuilder<>* t_builder, llvm::Value* t_lhs, llvm::Value* t_rhs){
            return t_builder->CreateFSub(t_lhs,t_rhs);
        }
    };
    const Operation MULT = {
        3,
        [](llvm::IRBuilder<>* t_builder, llvm::Value* t_lhs, llvm::Value* t_rhs){
            return t_builder->CreateFMul(t_lhs,t_rhs);
        }
    };
    const Operation DIV = {
        3,
        [](llvm::IRBuilder<>* t_builder, llvm::Value* t_lhs, llvm::Value* t_rhs){
            return t_builder->CreateFDiv(t_lhs,t_rhs);
        }
    };
    const Operation LESSER = {
        1,
        [](llvm::IRBuilder<>* t_builder, llvm::Value* t_lhs, llvm::Value* t_rhs){
            return t_builder->CreateUIToFP(
                t_builder->CreateFCmpULT(t_lhs,t_rhs),
                llvm::Type::getDoubleTy(t_builder->getContext())
            );
        }
    };
    const Operation GREATER = {
        1,
        [](llvm::IRBuilder<>* t_builder, llvm::Value* t_lhs, llvm::Value* t_rhs){
            return t_builder->CreateUIToFP(
                t_builder->CreateFCmpUGT(t_lhs,t_rhs),
                llvm::Type::getDoubleTy(t_builder->getContext())
            );
        } 
    };

    std::map<std::string,Operation> opKeys = {
        {"+",ADD},
        {"-",SUB},
        {"*",MULT},
        {"/",DIV},
        {"<",LESSER},
        {">",GREATER}
    };
}

Operation getOpFromKey(std::string t_key) {
    
}

#endif // TESTLANG_OPERATIONS_HPP