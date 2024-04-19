#ifndef TESTLANG_PARSER_HPP
#define TESTLANG_PARSER_HPP

#include "lexer.hpp"
#include "syntaxtree.hpp"
#include <cctype>
#include <iostream>

class Parser {
private:
  std::unique_ptr<Lexer> m_lexer;
  std::shared_ptr<Generator> m_genData;

  std::vector<std::unique_ptr<SyntaxTree>> parseBlock();
  std::unique_ptr<SyntaxTree> parseNum();
  std::unique_ptr<SyntaxTree> parseExpression();
  std::unique_ptr<SyntaxTree> parseParens();
  std::unique_ptr<SyntaxTree> parseIdentifier();
  std::unique_ptr<SyntaxTree> parseConditional();
  std::unique_ptr<SyntaxTree> handleUnknown();
  std::unique_ptr<SyntaxTree> parseMain();
  std::unique_ptr<SyntaxTree>
  parseOpRHS(const int t_minPrec, std::unique_ptr<SyntaxTree> t_leftSide);
  std::unique_ptr<PrototypeAST> parsePrototype();
  std::unique_ptr<SyntaxTree> parseReturn();
  std::unique_ptr<FunctionAST> parseDefinition();
  std::unique_ptr<PrototypeAST> parseExtern();
  std::unique_ptr<FunctionAST> parseTopLevel();

public:
  Parser(std::unique_ptr<Lexer> t_lexer, std::shared_ptr<Generator> t_genData)
      : m_lexer(std::move(t_lexer)), m_genData(t_genData) {}
  ~Parser() = default;

  bool operator()();
};

#endif // TESTLANG_PARSER_HPP