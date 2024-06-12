#ifndef BEAVER_PARSER_HPP
#define BEAVER_PARSER_HPP

#include "lexer.hpp"
#include "syntaxtree.hpp"
#include <cctype>
#include <iostream>

// Parsing an outer expression will return one of these
// Tells the main function how to continue
enum class ParserStatus { ok, end, error };

// Uses the lexer to parse the file into an AST
class Parser {
private:
  // Stored lexer
  std::unique_ptr<Lexer> m_lexer;

  // Stores a generator to pass it to code generation
  std::shared_ptr<Generator> m_genData;

  // Parse functions for various parts of the syntax
  std::optional<blockPtr> parseBlock();
  std::optional<expressionPtr> parseNum();
  std::optional<expressionPtr> parseExpression();
  std::optional<expressionPtr> parseParens();
  std::optional<expressionPtr> parseIdentifier();
  // returns 0 iff the block was successfully parsed
  bool parseConditionalBlock(
      std::vector<blockPtr> &mainBlocks,
      std::vector<expressionPtr> &conditions);
  std::optional<linePtr> parseConditional();
  std::optional<linePtr> parseWhile();
  std::optional<linePtr> parseFor();
  std::optional<expressionPtr> handleUnknown();
  std::optional<expressionPtr> parseMainExpr();
  std::optional<expressionPtr>
  parseOpRHS(const int t_minPrec, expressionPtr t_leftSide);
  std::optional<std::unique_ptr<PrototypeAST>> parsePrototype();
  std::optional<linePtr> parseReturn();
  std::optional<std::unique_ptr<FunctionAST>> parseDefinition();
  std::optional<std::unique_ptr<PrototypeAST>> parseExtern();
  std::optional<std::unique_ptr<FunctionAST>> parseTopLevel();
  std::optional<linePtr> parseInner();

public:
  Parser(std::unique_ptr<Lexer> t_lexer, std::shared_ptr<Generator> t_genData)
      : m_lexer(std::move(t_lexer)), m_genData(t_genData) {
    m_lexer->nextToken();
  }
  ~Parser() = default;

  ParserStatus parseOuter();
};

#endif // BEAVER_PARSER_HPP