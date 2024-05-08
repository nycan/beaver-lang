#ifndef TESTLANG_PARSER_HPP
#define TESTLANG_PARSER_HPP

#include "lexer.hpp"
#include "syntaxtree.hpp"
#include <cctype>
#include <iostream>

enum class ParserStatus { ok, end, error };

class Parser {
private:
  std::unique_ptr<Lexer> m_lexer;
  std::shared_ptr<Generator> m_genData;

  std::optional<std::vector<std::unique_ptr<SyntaxTree>>> parseBlock();
  std::optional<std::unique_ptr<SyntaxTree>> parseNum();
  std::optional<std::unique_ptr<SyntaxTree>> parseExpression();
  std::optional<std::unique_ptr<SyntaxTree>> parseParens();
  std::optional<std::unique_ptr<SyntaxTree>> parseIdentifier();
  std::optional<std::unique_ptr<SyntaxTree>> parseConditional();
  std::optional<std::unique_ptr<SyntaxTree>> handleUnknown();
  std::optional<std::unique_ptr<SyntaxTree>> parseMain();
  std::optional<std::unique_ptr<SyntaxTree>>
  parseOpRHS(const int t_minPrec, std::unique_ptr<SyntaxTree> t_leftSide);
  std::optional<std::unique_ptr<PrototypeAST>> parsePrototype();
  std::optional<std::unique_ptr<SyntaxTree>> parseReturn();
  std::optional<std::unique_ptr<FunctionAST>> parseDefinition();
  std::optional<std::unique_ptr<PrototypeAST>> parseExtern();
  std::optional<std::unique_ptr<FunctionAST>> parseTopLevel();

public:
  Parser(std::unique_ptr<Lexer> t_lexer, std::shared_ptr<Generator> t_genData)
      : m_lexer(std::move(t_lexer)), m_genData(t_genData) {
    m_lexer->nextToken();
  }
  ~Parser() = default;

  ParserStatus parseOuter();
};

#endif // TESTLANG_PARSER_HPP