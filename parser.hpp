#ifndef TESTLANG_PARSER__HPP
#define TESTLANG_PARSER_HPP

#include "lexer.hpp"
#include "syntaxtree.hpp"
#include <iostream>
#include <cctype>
#include <map>

class Parser{
private:
    static Lexer m_lexer;

public:
    static std::unique_ptr<SyntaxTree> parseNum(){
        auto result = std::make_unique<NumberAST>(m_lexer.getNum());
        m_lexer.nextToken();
        return std::move(result);
    }

    static std::unique_ptr<SyntaxTree> parseExpression();

    static std::unique_ptr<SyntaxTree> parseParens(){
        m_lexer.nextToken();
        auto exprResult = parseExpression();
        if (!exprResult){
            return nullptr;
        }
        if(m_lexer.getChar() != ')'){
            std::cerr << "Missing ')'\n";
            return nullptr;
        }
    }

    static std::unique_ptr<SyntaxTree> parseIdentifier(){
        std::string idName = m_lexer.getIdentifier();
        m_lexer.nextToken();

        //variable
        if(m_lexer.getChar() != '('){
            return std::make_unique<SyntaxTree>(idName);
        }

        //function call
        m_lexer.nextToken();
        std::vector<std::unique_ptr<SyntaxTree>> args;
        if(m_lexer.getChar() != ')'){
            while(true){
                if(auto argument = parseExpression()){
                    args.push_back(std::move(argument));
                } else {
                    return nullptr;
                }
                
                if(m_lexer.getChar() == ')'){
                    break;
                }

                if(m_lexer.getChar() != ','){
                    std::cerr << "Expected ')' or ',' in argument list.";
                    return nullptr;
                }
                m_lexer.nextToken();
            }
        }

        m_lexer.nextToken();
        return std::make_unique<CallAST>(idName, std::move(args));
    }

    static std::unique_ptr<SyntaxTree> handleUnknown(){
        switch(m_lexer.getChar()){
            case '(':
                return parseParens();
            default:
                std::cerr << "Unknown token\n";
                return nullptr;
        }
    }

    static std::unique_ptr<SyntaxTree> parseMain(){
        switch(m_lexer.getTok()){
            case Token::IDENTIFIER:
                return parseIdentifier();
            case Token::NUMBER:
                return parseNum();
            default:
                return handleUnknown();
        }
    }

    static int BinopPrecedence(){ //TODO: replace this with something better
        switch(m_lexer.getChar()){
            case '*':
                return 3;
            case '/':
                return 3;
            case '+':
                return 2;
            case '-':
                return 2;
            case '<':
                return 1;
            case '>':
                return 1;
            default:
                return -1;
        }
    }

    static std::unique_ptr<SyntaxTree> parseOpRHS(
        const int t_minPrec,
        std::unique_ptr<SyntaxTree> t_leftSide
    ){
        while(true){
            int precedence = BinopPrecedence();

            //if it is lower precedence, then it will be parsed in a different call
            if(precedence < t_minPrec){
                return t_leftSide;
            }

            int operation = m_lexer.getChar();
            m_lexer.nextToken();

            auto rightSide = parseMain();
            if(!rightSide){
                return nullptr;
            }

            int nextPrec = BinopPrecedence();
            if(precedence < nextPrec){
                rightSide = parseOpRHS(precedence+1, std::move(rightSide));
                if(!rightSide){
                    return nullptr;
                }
            }

            t_leftSide = std::make_unique<BinaryOpAST>(
                operation,std::move(t_leftSide),
                std::move(rightSide)
            );
        }
    }

    static std::unique_ptr<SyntaxTree> parseExpression(){
        auto leftSide = parseMain();
        if(!leftSide){
            return nullptr;
        }
        return parseOpRHS(0, std::move(leftSide));
    }

    static std::unique_ptr<PrototypeAST> parsePrototype(){
        if(m_lexer.getTok() != Token::IDENTIFIER){
            std::cerr << "Expected function name in prototype.\n";
            return nullptr;
        }
        
        //function name
        std::string funcName = m_lexer.getIdentifier();
        m_lexer.nextToken();

        //arguments
        if(m_lexer.getChar() != '('){
            std::cerr << "Expected '('\n";
        }
        std::vector<std::string> args;
        while(m_lexer.nextToken() == Token::IDENTIFIER){
            args.push_back(m_lexer.getIdentifier());
        }
        if(m_lexer.getChar() != ')'){
            std::cerr << "Expected ')'\n";
        }
        m_lexer.nextToken();

        return std::make_unique<PrototypeAST>(funcName,std::move(args));
    }

    static std::unique_ptr<FunctionAST> parseDefinition(){
        //function declaration
        m_lexer.nextToken();

        //prototype
        auto prototype = parsePrototype();
        if(!prototype){
            return nullptr;
        }

        if(auto expr = parseExpression()){
            return std::make_unique<FunctionAST>(std::move(prototype),std::move(expr));
        }
        return nullptr;
    }
};

#endif