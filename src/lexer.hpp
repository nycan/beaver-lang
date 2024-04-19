#ifndef TESTLANG_LEXER_HPP
#define TESTLANG_LEXER_HPP

#include <cctype>
#include <fstream>
#include <string>
#include <iostream>

// all tokens
enum class Token {
  unknown,
  endFile,
  func,
  externTok,
  identifier,
  number,
  ifTok,
  elseTok,
  returnTok
};

class Lexer {
protected:
  // for error handling
  unsigned m_lineNumber;
  unsigned m_charPos;

  // way to get the next character depends on input method
  virtual char processChar() = 0;

private:
  // stored processed values
  std::string m_identifier;
  double m_numVal;
  std::string m_operation;

  // information
  Token m_currTok;
  char m_currChar;
  char m_lastChar;

  // gives the lexer the next character, updates variables
  char nextChar();

  // read the next token
  Token processToken();

public:
  Lexer()
      : m_identifier(""), m_numVal(0), m_operation(""), m_currTok(Token::unknown),
        m_currChar(' '), m_lastChar(' '), m_lineNumber(1), m_charPos(1) {}
  ~Lexer() = default;

  inline std::string getIdentifier() const { return m_identifier; }
  inline double getNum() const { return m_numVal; }
  inline std::string getOperation() const { return m_operation; }
  inline Token getTok() const { return m_currTok; }
  inline char getChar() const { return m_lastChar; }

  inline unsigned getLine() const { return m_lineNumber; }
  inline unsigned getPos() const { return m_charPos; }

  // almost the same as processToken(), but also update m_currTok.
  inline Token nextToken() { return m_currTok = processToken(); }
};

// with input file
class FileLexer : public Lexer {
private:
  std::ifstream m_fileStream;

protected:
  inline char processChar() override { return m_fileStream.get(); }

public:
  FileLexer(std::string t_fileName) : Lexer(), m_fileStream(t_fileName) {}
  ~FileLexer() = default;
};

// read from stdin
class StdinLexer : public Lexer {
protected:
  inline char processChar() override { return getchar(); }

public:
  StdinLexer() : Lexer() {}
  ~StdinLexer() = default;
};

#endif // TESTLANG_LEXER_HPP