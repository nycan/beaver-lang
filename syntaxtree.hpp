#ifndef TESTLANG_SYNTAXTREE_HPP
#define TESTLANG_SYNTAXTREE_HPP

#include <string>
#include <vector>
#include <memory>

class SyntaxTree{
public:
    virtual ~SyntaxTree() = default;
};

class NumberAST : public SyntaxTree{
private:
    double m_value;

public:
    NumberAST(const double t_value): m_value(t_value) {}
};

class VariableAST : public SyntaxTree{
private:
    std::string m_name;

public:
    VariableAST(const std::string& t_name): m_name(t_name) {}
};

class BinaryOpAST : public SyntaxTree{
private:
    char m_op;
    std::unique_ptr<SyntaxTree> m_lhs, m_rhs;

public:
    BinaryOpAST(const char t_op,
                     std::unique_ptr<SyntaxTree> t_lhs,
                     std::unique_ptr<SyntaxTree> t_rhs
                     ): m_op(t_op), m_lhs(std::move(t_lhs)), m_rhs(std::move(t_rhs)) {}
};

class CallAST : public SyntaxTree{
private:
    std::string m_callee;
    std::vector<std::unique_ptr<SyntaxTree>> m_args;

public:
    CallAST(const std::string& t_callee,
                       std::vector<std::unique_ptr<SyntaxTree>> t_args
                       ): m_callee(t_callee), m_args(std::move(t_args)) {}
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