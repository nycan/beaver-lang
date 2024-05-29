#ifndef BEAVER_LEXER_HPP
#define BEAVER_LEXER_HPP

#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

// Class for all special tokens
enum class Token {
  unknown,
  endFile,
  func,
  externTok,
  identifier,
  number,
  ifTok,
  elseTok,
  returnTok,
  whileTok,
  forTok
};

namespace lexer {
// conversion from strings to tokens
const std::map<std::string, Token> TokenKeys = {{"fn", Token::func},
                                                {"extern", Token::externTok},
                                                {"if", Token::ifTok},
                                                {"else", Token::elseTok},
                                                {"ret", Token::returnTok},
                                                {"while", Token::whileTok},
                                                {"for", Token::forTok}};
} // namespace lexer

// returns the token if one is found, and Token::identifier otherwise
Token getTokFromKey(std::string t_key);

// Responsible for reading the file and parsing into tokens
class Lexer {
protected:
  // for error handling
  // Note: currently somewhat buggy
  unsigned m_lineNumber;
  unsigned m_charPos;

  // Read the next character
  // Implementation specified in derived class
  virtual char processChar() = 0;

private:
  // stored processed values
  std::string m_identifier;
  double m_numVal;
  std::string m_operation;

  // Character information
  Token m_currTok;
  char m_currChar;
  char m_lastChar;

  // gives the lexer the next character, updates variables
  char nextChar();

  // Does the actual processing for tokens
  Token processToken();

public:
  Lexer()
      : m_identifier(""), m_numVal(0), m_operation(""),
        m_currTok(Token::unknown), m_currChar(' '), m_lastChar(' '),
        m_lineNumber(1), m_charPos(1) {}
  virtual ~Lexer() = default;

  // Functions to get stored information

  inline std::string getIdentifier() const { return m_identifier; }
  inline double getNum() const { return m_numVal; }
  inline std::string getOperation() const { return m_operation; }
  inline Token getTok() const { return m_currTok; }
  inline char getChar() const { return m_lastChar; }

  // Line and position functions

  inline unsigned getLine() const { return m_lineNumber; }
  inline unsigned getPos() const { return m_charPos; }

  // Process the next token
  inline Token nextToken() { return m_currTok = processToken(); }
};

// For reading from files
class FileLexer : public Lexer {
private:
  std::ifstream m_fileStream;

protected:
  inline char processChar() override { return m_fileStream.get(); }

public:
  FileLexer(std::string t_fileName) : Lexer(), m_fileStream(t_fileName) {}
  ~FileLexer() = default;
};

// Read from stdin
// Currently not used
class StdinLexer : public Lexer {
protected:
  inline char processChar() override { return getchar(); }

public:
  StdinLexer() : Lexer() {}
  ~StdinLexer() = default;
};

#endif // BEAVER_LEXER_HPP