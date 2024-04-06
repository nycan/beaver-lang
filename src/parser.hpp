#ifndef TESTLANG_PARSER_HPP
#define TESTLANG_PARSER_HPP

#include "lexer.hpp"
#include "syntaxtree.hpp"
#include <iostream>
#include <cctype>
#include <map>

class Parser{
private:
    Lexer m_lexer;
    std::unique_ptr<GeneratorData> m_genData;

public:
    Parser(Lexer& t_lexer): m_lexer(t_lexer) {}
    std::unique_ptr<SyntaxTree> parseNum();
    std::unique_ptr<SyntaxTree> parseExpression();
    std::unique_ptr<SyntaxTree> parseParens();
    std::unique_ptr<SyntaxTree> parseIdentifier();
    std::unique_ptr<SyntaxTree> handleUnknown();
    std::unique_ptr<SyntaxTree> parseMain();
    int BinopPrecedence();
    std::unique_ptr<SyntaxTree> parseOpRHS(
        const int t_minPrec,
        std::unique_ptr<SyntaxTree> t_leftSide
    );
    std::unique_ptr<PrototypeAST> parsePrototype();
    std::unique_ptr<FunctionAST> parseDefinition();
    std::unique_ptr<PrototypeAST> parseExtern();
    std::unique_ptr<FunctionAST> parseTopLevel();
    void operator()();
};

std::unique_ptr<SyntaxTree> Parser::parseNum(){
    auto result = std::make_unique<NumberAST>(m_lexer.getNum());
    m_lexer.nextToken();
    return std::move(result);
}

std::unique_ptr<SyntaxTree> Parser::parseParens(){
    m_lexer.nextToken();
    auto exprResult = parseExpression();
    if (!exprResult){
        return nullptr;
    }
    if(m_lexer.prevChar() != ')'){
        fprintf(stderr,"Missing ')'\n");
        return nullptr;
    }
    m_lexer.nextToken();
    return exprResult;
}

std::unique_ptr<SyntaxTree> Parser::parseIdentifier(){
    std::string idName = m_lexer.getIdentifier();
    m_lexer.nextToken();

    //variable
    if(m_lexer.prevChar() != '('){
        return std::make_unique<VariableAST>(idName);
    }

    //function call
    m_lexer.nextToken();
    std::vector<std::unique_ptr<SyntaxTree>> args;
    if(m_lexer.prevChar() != ')'){
        while(true){
            if(auto argument = parseExpression()){
                args.push_back(std::move(argument));
            } else {
                return nullptr;
            }
            
            if(m_lexer.prevChar() == ')'){
                break;
            }

            if(m_lexer.prevChar() != ','){
                fprintf(stderr,"Expected ')' or ',' in argument list.");
                return nullptr;
            }
            m_lexer.nextToken();
        }
    }

    m_lexer.nextToken();
    return std::make_unique<CallAST>(idName, std::move(args));
}

std::unique_ptr<SyntaxTree> Parser::handleUnknown(){
    switch(m_lexer.prevChar()){
        case '(':
            return parseParens();
        default:
            std::cerr << m_lexer.prevChar() << '\n';
            fprintf(stderr,"Unknown token\n");
            return nullptr;
    }
}

std::unique_ptr<SyntaxTree> Parser::parseMain(){
    switch(m_lexer.getTok()){
        case Token::IDENTIFIER:
            return parseIdentifier();
        case Token::NUMBER:
            return parseNum();
        default:
            return handleUnknown();
    }
}

int Parser::BinopPrecedence(){ //TODO: replace this with something better
    switch(m_lexer.prevChar()){
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

std::unique_ptr<SyntaxTree> Parser::parseOpRHS(
    const int t_minPrec,
    std::unique_ptr<SyntaxTree> t_leftSide
){
    while(true){
        int precedence = BinopPrecedence();

        //if it is lower precedence, then it will be parsed in a different call
        if(precedence < t_minPrec){
            return t_leftSide;
        }

        char operation = m_lexer.prevChar();
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
            operation,
            std::move(t_leftSide),
            std::move(rightSide)
        );
    }
}

std::unique_ptr<SyntaxTree> Parser::parseExpression(){
    auto leftSide = parseMain();
    if(!leftSide){
        return nullptr;
    }
    return parseOpRHS(0, std::move(leftSide));
}

std::unique_ptr<PrototypeAST> Parser::parsePrototype(){
    if(m_lexer.getTok() != Token::IDENTIFIER){
        fprintf(stderr,"Expected function name in prototype.\n");
        return nullptr;
    }
    
    //function name
    std::string funcName = m_lexer.getIdentifier();
    m_lexer.nextToken();

    //arguments
    if(m_lexer.prevChar() != '('){
        fprintf(stderr,"Expected '('\n");
        return nullptr;
    }
    std::vector<std::string> args;
    while(m_lexer.nextToken() == Token::IDENTIFIER){
        args.push_back(m_lexer.getIdentifier());
    }
    if(m_lexer.prevChar() != ')'){
        fprintf(stderr,"Expected ')'\n");
        return nullptr;
    }
    m_lexer.nextToken();

    return std::make_unique<PrototypeAST>(funcName,std::move(args));
}

std::unique_ptr<FunctionAST> Parser::parseDefinition(){
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

std::unique_ptr<PrototypeAST> Parser::parseExtern(){
    m_lexer.nextToken();
    return parsePrototype();
}

std::unique_ptr<FunctionAST> Parser::parseTopLevel(){
    if(auto expr = parseExpression()){
        auto prototype = std::make_unique<PrototypeAST>(
            "somethingThatIllProbablyForgetToChange",
            std::vector<std::string>()
        );
        return std::make_unique<FunctionAST>(std::move(prototype),std::move(expr));
    }
    return nullptr;
}

//temporary main function
void Parser::operator()(){
    m_lexer.nextToken();    

    while(true){
        switch(m_lexer.getTok()){
            case Token::ENDFILE:
                return;
            case Token::FUNC:
                if(parseDefinition()){
                    fprintf(stderr,"Successfully parsed function definition.\n");
                } else {
                    m_lexer.nextToken();
                }
                break;
            case Token::EXTERN:
                if(parseExtern()){
                    fprintf(stderr,"Successfully parsed extern.\n");
                } else {
                    m_lexer.nextToken();
                }
                break;
            default:
                if(m_lexer.prevChar()==';'){
                    m_lexer.nextToken();
                } else {
                    if(parseTopLevel()){
                        fprintf(stderr,"Successfully parsed top-level expression.\n");
                    } else {
                        m_lexer.nextToken();
                    }
                }
                break;
        }
    }
}

#endif