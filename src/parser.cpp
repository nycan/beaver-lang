#include "parser.hpp"

// helper function for blocks
std::optional<std::vector<std::unique_ptr<SyntaxTree>>> Parser::parseBlock() {
  // parse '{'
  if (m_lexer->getChar() != '{') {
    llvm::errs() << "Expected '{'.";
    return {};
  }
  m_lexer->nextToken();

  // parse body
  std::vector<std::unique_ptr<SyntaxTree>> result;
  while (m_lexer->getChar() != '}') {
    if (m_lexer->getChar() == EOF) {
      llvm::errs() << "Expected '}'.";
      return {};
    }
    if (auto line = parseMain()) {
      result.push_back(std::move(*line));
    } else {
      return {};
    }
    m_lexer->nextToken();
  }

  // parse '}'
  m_lexer->nextToken();
  return result;
}

std::optional<std::unique_ptr<SyntaxTree>> Parser::parseNum() {
  auto result = std::make_unique<NumberAST>(m_genData, m_lexer->getNum());
  m_lexer->nextToken();
  return std::move(result);
}

std::optional<std::unique_ptr<SyntaxTree>> Parser::parseParens() {
  // parse '('
  m_lexer->nextToken();

  // parse inside expression
  auto exprResult = parseExpression();

  // parse ')'
  if (m_lexer->getChar() != ')') {
    llvm::errs() << "Missing ')'\n";
    return {};
  }
  m_lexer->nextToken();

  return exprResult;
}

std::optional<std::unique_ptr<SyntaxTree>> Parser::parseIdentifier() {
  // parse identifier
  std::string idName = m_lexer->getIdentifier();
  m_lexer->nextToken();

  // variable
  if (m_lexer->getChar() != '(') {
    return std::make_unique<VariableAST>(m_genData, idName);
  }

  // function call
  m_lexer->nextToken();
  std::vector<std::unique_ptr<SyntaxTree>> args;
  while (m_lexer->getChar() != ')') {
    // parse argument
    if (auto argument = parseExpression()) {
      args.push_back(std::move(*argument));
    } else {
      return {};
    }

    // end of arg list
    if (m_lexer->getChar() == ')') {
      break;
    }

    // separator
    if (m_lexer->getChar() != ',') {
      llvm::errs() << "Expected ')' or ',' in argument list.\n";
      return {};
    }
    m_lexer->nextToken();
  }

  // parse ')'
  m_lexer->nextToken();
  return std::make_unique<CallAST>(m_genData, idName, std::move(args));
}

std::optional<std::unique_ptr<SyntaxTree>> Parser::parseConditional() {
  // parse 'if'
  m_lexer->nextToken();

  // parse condition
  auto condition = parseExpression();
  if (!condition) {
    return {};
  }

  // parse main block
  auto mainBlock = parseBlock();
  if (!mainBlock) {
    return {};
  }

  // if an else block exists, parse it
  std::optional<std::vector<std::unique_ptr<SyntaxTree>>> elseBlock;
  if (m_lexer->getTok() == Token::elseTok) {
    // parse "else"
    m_lexer->nextToken();

    elseBlock = parseBlock();
    if (!elseBlock) {
      return {};
    }
  }

  return std::make_unique<ConditionalAST>(m_genData, std::move(*condition),
                                          std::move(*mainBlock),
                                          std::move(elseBlock));
}

// helper function for parseMain to parse the last character when the token is
// unknown
std::optional<std::unique_ptr<SyntaxTree>> Parser::handleUnknown() {
  switch (m_lexer->getChar()) {
  case '(':
    return parseParens();
  case ';':
    m_lexer->nextToken();
    return parseMain();
  default:
    llvm::errs() << "Unknown token: " << m_lexer->getChar() <<  '\n';
    return {};
  }
}

// main parse function
std::optional<std::unique_ptr<SyntaxTree>> Parser::parseMain() {
  switch (m_lexer->getTok()) {
  case Token::identifier:
    return parseIdentifier();
  case Token::number:
    return parseNum();
  case Token::ifTok:
    return parseConditional();
  case Token::returnTok:
    return parseReturn();
  case Token::elseTok:
    llvm::errs() << "Got 'else' with no 'if' to match.\n";
    return {};
  default:
    return handleUnknown();
  }
}

std::optional<std::unique_ptr<SyntaxTree>>
Parser::parseOpRHS(const int t_minPrec,
                   std::unique_ptr<SyntaxTree> t_leftSide) {
  while (true) {
    // parse operation
    auto op = getOpFromKey(m_lexer->getOperation());
    if(!op.has_value()) {
      return t_leftSide;
    }
    m_lexer->nextToken();

    // if it is lower precedence, then it will be parsed in a different call
    if (op->precedence < t_minPrec) {
      return t_leftSide;
    }

    // parse right side
    auto rightSide = parseMain();
    if (!rightSide) {
      return {};
    }

    // if the expression continues, parse it
    auto nextOp = getOpFromKey(m_lexer->getOperation());
    if(!nextOp.has_value()){
      return std::make_unique<BinaryOpAST>(
        m_genData, *op, std::move(t_leftSide), std::move(*rightSide));
    }
    // if the next operator is higher precedence, it needs to be handled before
    // this one parse recursively
    
    if (op->precedence < nextOp->precedence) {
      rightSide = parseOpRHS(op->precedence + 1, std::move(*rightSide));
      if (!rightSide) {
        return {};
      }
    }

    t_leftSide = std::make_unique<BinaryOpAST>(
        m_genData, *op, std::move(t_leftSide), std::move(*rightSide));
  }
}

std::optional<std::unique_ptr<SyntaxTree>> Parser::parseExpression() {
  auto leftSide = parseMain();
  if (!leftSide) {
    return {};
  }
  return parseOpRHS(0, std::move(*leftSide));
}

std::optional<std::unique_ptr<PrototypeAST>> Parser::parsePrototype() {
  if (m_lexer->getTok() != Token::identifier) {
    llvm::errs() << "Expected function name in prototype.\n";
    return {};
  }

  // function name
  std::string funcName = m_lexer->getIdentifier();
  m_lexer->nextToken();

  // arguments
  if (m_lexer->getChar() != '(') {
    llvm::errs() << "Expected '('\n";
    return {};
  }
  m_lexer->nextToken();
  std::vector<std::string> args;
  while (m_lexer->getChar() != ')') {
    // parse argument
    if (m_lexer->getTok() != Token::identifier) {
      llvm::errs() << "Unexpected token in prototype\n";
      return {};
    }
    args.push_back(m_lexer->getIdentifier());

    // end of arg list
    m_lexer->nextToken();
    if (m_lexer->getChar() == ')') {
      break;
    }

    // separator
    if (m_lexer->getChar() != ',') {
      llvm::errs() << "Expected ')' or ',' in parameter list.\n";
      return {};
    }
    m_lexer->nextToken();
  }

  // parse ')'
  if (m_lexer->getChar() != ')') {
    llvm::errs() << "Expected ')'\n";
    return {};
  }
  m_lexer->nextToken();

  return std::make_unique<PrototypeAST>(m_genData, funcName, std::move(args));
}

std::optional<std::unique_ptr<SyntaxTree>> Parser::parseReturn() {
  // parse "ret"
  m_lexer->nextToken();

  if (auto resAST = parseMain()) {
    return std::make_unique<ReturnAST>(m_genData, std::move(*resAST));
  }
  return {};
}

std::optional<std::unique_ptr<FunctionAST>> Parser::parseDefinition() {
  // function declaration
  m_lexer->nextToken();

  // prototype
  auto prototype = parsePrototype();
  if (!prototype) {
    return {};
  }

  // body
  if (auto block = parseBlock()) {
    return std::make_unique<FunctionAST>(m_genData, std::move(*prototype),
                                         std::move(*block));
  }

  return {};
}

std::optional<std::unique_ptr<PrototypeAST>> Parser::parseExtern() {
  m_lexer->nextToken();
  return parsePrototype();
}

// wrap top-level expressions in an anonymous prototype
std::optional<std::unique_ptr<FunctionAST>> Parser::parseTopLevel() {
  if (auto expr = parseExpression()) {
    auto prototype = std::make_unique<PrototypeAST>(
        m_genData,
        "somethingThatIllProbablyForgetToChange", // Don't forget to change this
        std::vector<std::string>());
    std::vector<std::unique_ptr<SyntaxTree>> exprBlock;
    exprBlock.push_back(std::move(*expr));
    return std::make_unique<FunctionAST>(m_genData, std::move(prototype),
                                         std::move(exprBlock));
  }
  return {};
}

// temporary main function
bool Parser::operator()() {
  m_lexer->nextToken();
  bool hasErrors = false;

  while (true) {
    switch (m_lexer->getTok()) {
    case Token::endFile:
      return false;
    case Token::func:
      if (auto resAST = parseDefinition()) {
        if (auto *resIR = (*resAST)->codegen()) {
          std::cerr << "Successfully parsed function definition.\n";
        } else {
          hasErrors = true;
        }
      } else {
        hasErrors = true;
        m_lexer->nextToken();
      }
      break;
    case Token::externTok:
      if (auto resAST = parseExtern()) {
        if (auto *resIR = (*resAST)->codegen()) {
          std::cerr << "Successfully parsed extern.\n";
        } else {
          hasErrors = true;
        }
      } else {
        hasErrors = true;
        m_lexer->nextToken();
      }
      break;
    default:
      if (m_lexer->getChar() == ';') {
        m_lexer->nextToken();
      } else {
        if (auto resAST = parseTopLevel()) {
          if (auto *resIR = (*resAST)->codegen()) {
            std::cerr << "Successfully parsed top-level expression.\n";
            resIR->removeFromParent();
          } else {
            hasErrors = true;
          }
        } else {
          hasErrors = true;
          m_lexer->nextToken();
        }
      }
      break;
    }
  }
  return hasErrors;
}