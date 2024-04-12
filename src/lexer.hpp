#ifndef TESTLANG_LEXER_HPP
#define TESTLANG_LEXER_HPP

#include <string>
#include <cctype>
#include <fstream>

// all tokens
enum class Token{
    UNKNOWN,
    ENDFILE,
    FUNC,
    EXTERN,
    IDENTIFIER,
    NUMBER,
    IF,
    ELSE,
    RETURN
};

class Lexer{
protected:
    // for error handling
    unsigned m_lineNumber;
    unsigned m_charPos;

    // way to get the next character depends on input method
    virtual char processChar() {return getchar();};

private:
    // stored processed values
    std::string m_identifier;
    double m_numVal;

    // information
    Token m_currTok;
    char m_currChar;
    char m_lastChar;

    // gives the lexer the next character, updates variables
    char nextChar(){
        m_lastChar = m_currChar;
        m_currChar = processChar();
        if(m_currChar == '\n'){
            ++m_lineNumber;
            m_charPos = 1;
        } else {
            ++m_charPos;
        }
        return m_currChar;
    }

    //read the next token
    Token processToken(){
        //ignore whitespace
        while(std::isspace(m_currChar)){
            nextChar();
        }

        // Keywords and identifiers
        if(std::isalpha(m_currChar)){
            m_identifier = m_currChar;
            // [A-z]([A-z]|[1-9])*
            while(std::isalnum(nextChar())){
                m_identifier += m_currChar;
            }

            //keywords
            if(m_identifier=="fn"){
                return Token::FUNC;
            }
            if(m_identifier=="extern"){
                return Token::EXTERN;
            }
            if(m_identifier=="if"){
                return Token::IF;
            }
            if(m_identifier=="else"){
                return Token::ELSE;
            }
            if(m_identifier=="ret"){
                return Token::RETURN;
            }
            // if not a keyword, it's an identifier
            return Token::IDENTIFIER;
        }

        // Numbers
        if(std::isdigit(m_currChar) || m_currChar=='.'){
            std::string numStr;
            do{
                numStr += m_currChar;
                nextChar();
            } while (std::isdigit(m_currChar) || m_currChar=='.');

            m_numVal = std::stoi(numStr);
            return Token::NUMBER;
        }

        // Comments
        if(m_currChar=='#'){
            do{
                nextChar();
            } while (m_currChar != EOF && m_currChar != '\n' && m_currChar != '\r');

            if(m_currChar != EOF){
                return nextToken();
            }
        }

        if(m_currChar==EOF){
            return Token::ENDFILE;
        }

        nextChar();
        return Token::UNKNOWN;
    }

public:
    Lexer(): m_identifier(""), m_numVal(0), 
             m_currTok(Token::UNKNOWN), m_currChar(getchar()), m_lastChar(' '),
             m_lineNumber(1), m_charPos(1) {}
    ~Lexer() = default;

    inline std::string getIdentifier() const {return m_identifier;}
    inline double getNum() const {return m_numVal;}
    inline Token getTok() const {return m_currTok;}
    inline char getChar() const {return m_lastChar;}

    inline unsigned getLine() const {return m_lineNumber;}
    inline unsigned getPos() const {return m_charPos;}

    //almost the same as processToken(), but also update m_currTok.
    inline Token nextToken(){
        return m_currTok = processToken();
    }
};

// with input file
class FileLexer : public Lexer {
private:
    std::ifstream m_fileStream;

protected:
    inline char processChar() override {
        return m_fileStream.get();
    }

public:
    FileLexer(std::string t_fileName): Lexer(), m_fileStream(t_fileName) {}
    ~FileLexer() = default;
};

// read from stdin
class StdinLexer : public Lexer {
protected:
    inline char processChar() override {
        return getchar();
    }

public:
    StdinLexer(): Lexer() {}
    ~StdinLexer() = default;
};

#endif // TESTLANG_LEXER_HPP