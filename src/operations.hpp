#ifndef TESTLANG_OPERATIONS_HPP
#define TESTLANG_OPERATIONS_HPP

#include "llvm/IR/IRBuilder.h"
#include "generator.hpp"
#include <string>
#include <map>

// defines an arbitrary operation
// everything is public since they will all be constant
struct Operation {
    const int precedence;
    llvm::Value* (*codegen)(std::shared_ptr<Generator>, llvm::Value*, llvm::Value*);
};

namespace operations{
    const Operation ADD = {
        2,
        [](std::shared_ptr<Generator> t_gen, llvm::Value* t_lhs, llvm::Value* t_rhs){
            return t_gen->m_builder.CreateFAdd(t_lhs,t_rhs);
        }
    };
    const Operation SUB = {
        2,
        [](std::shared_ptr<Generator> t_gen, llvm::Value* t_lhs, llvm::Value* t_rhs){
            return t_gen->m_builder.CreateFSub(t_lhs,t_rhs);
        }
    };
    const Operation MULT = {
        3,
        [](std::shared_ptr<Generator> t_gen, llvm::Value* t_lhs, llvm::Value* t_rhs){
            return t_gen->m_builder.CreateFMul(t_lhs,t_rhs);
        }
    };
    const Operation DIV = {
        3,
        [](std::shared_ptr<Generator> t_gen, llvm::Value* t_lhs, llvm::Value* t_rhs){
            return t_gen->m_builder.CreateFDiv(t_lhs,t_rhs);
        }
    };
    const Operation LESSER = {
        1,
        [](std::shared_ptr<Generator> t_gen, llvm::Value* t_lhs, llvm::Value* t_rhs){
            return t_gen->m_builder.CreateUIToFP(
                t_gen->m_builder.CreateFCmpULT(t_lhs,t_rhs),
                llvm::Type::getDoubleTy(t_gen->m_builder.getContext())
            );
        }
    };
    const Operation GREATER = {
        1,
        [](std::shared_ptr<Generator> t_gen, llvm::Value* t_lhs, llvm::Value* t_rhs){
            return t_gen->m_builder.CreateUIToFP(
                t_gen->m_builder.CreateFCmpUGT(t_lhs,t_rhs),
                llvm::Type::getDoubleTy(t_gen->m_builder.getContext())
            );
        } 
    };

    const std::map<std::string,Operation> opKeys = {
        {"+",ADD},
        {"-",SUB},
        {"*",MULT},
        {"/",DIV},
        {"<",LESSER},
        {">",GREATER}
    };
}

// find operation given text
Operation getOpFromKey(std::string t_key);

#endif // TESTLANG_OPERATIONS_HPP