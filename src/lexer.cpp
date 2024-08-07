#include "lexer.hpp"

Token getTokFromKey(std::string t_key) {
  auto lookup = lexer::TokenKeys.find(t_key);
  if (lookup == lexer::TokenKeys.end()) {
    return Token::identifier;
  }
  return lookup->second;
}

char Lexer::nextChar() {
  m_lastChar = m_currChar;
  m_currChar = processChar();
  if (m_currChar == '\n') {
    ++m_lineNumber;
    m_charPos = 0;
  } else {
    ++m_charPos;
  }
  return m_currChar;
}

// Kind of a band-aid function, will probably be replaced later
// Returns whether a character is a valid operation character
bool isOperation(char t_character) {
  if (std::isalnum(t_character)) {
    return false;
  }
  if (std::isspace(t_character)) {
    return false;
  }
  static const char invalid[8] = {'(', ')', '{', '}', '[', ']', ';', ','};
  for (int idx = 0; idx < 8; ++idx) {
    if (t_character == invalid[idx]) {
      return false;
    }
  }
  return true;
}

Token Lexer::processToken() {
  // ignore whitespace
  while (std::isspace(m_currChar)) {
    nextChar();
  }

  // Keywords and identifiers
  if (std::isalpha(m_currChar)) {
    m_identifier = m_currChar;
    // [A-z]([A-z]|[1-9])*
    while (std::isalnum(nextChar())) {
      m_identifier += m_currChar;
    }

    return getTokFromKey(m_identifier);
  }

  // Numbers
  if (std::isdigit(m_currChar) || m_currChar == '.') {
    std::string numStr;
    do {
      numStr += m_currChar;
      nextChar();
    } while (std::isdigit(m_currChar) || m_currChar == '.');

    m_numVal = std::stoi(numStr);
    return Token::number;
  }

  // Comments
  if (m_currChar == '#') {
    do {
      nextChar();
    } while (m_currChar != EOF && m_currChar != '\n' && m_currChar != '\r');

    if (m_currChar != EOF) {
      return nextToken();
    }
  }

  if (m_currChar == EOF) {
    return Token::endFile;
  }

  // Operations
  if (!isOperation(m_currChar)) {
    m_operation = "";
    nextChar();
    return Token::unknown;
  }
  m_operation = m_currChar;
  while (isOperation(nextChar())) {
    m_operation += m_currChar;
  }

  // If all else fails, return unknown
  return Token::unknown;
}