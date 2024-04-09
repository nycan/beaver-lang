#ifndef TESTLANG_LEXER_HPP
#define TESTLANG_LEXER_HPP

#include <string>
#include <cctype>

// all tokens
enum class Token{
    UNKNOWN,
    ENDFILE,
    FUNC,
    EXTERN,
    IDENTIFIER,
    NUMBER,
    IF,
    ELSE
};

class Lexer{
private:
    // stored processed values
    std::string m_identifier;
    double m_numVal;

    // information
    Token m_currTok;
    char m_currChar;
    char m_lastChar;

public:
    Lexer(): m_identifier(""), m_numVal(0), 
             m_currTok(Token::UNKNOWN), m_currChar(getchar()), m_lastChar(' ') {}
    ~Lexer() = default;

    inline std::string getIdentifier() const {return m_identifier;}
    inline double getNum() const {return m_numVal;}
    inline Token getTok() const {return m_currTok;}
    inline char getChar() const {return m_currChar;}
    inline char prevChar() const {return m_lastChar;}

    // gives the lexer the next character, updates variables
    char nextchar(){
        m_lastChar = m_currChar;
        return m_currChar = getchar();
    }

    //read the next token
    Token processToken(){
        //ignore whitespace
        while(std::isspace(m_currChar)){
            nextchar();
        }

        // Keywords and identifiers
        if(std::isalpha(m_currChar)){
            m_identifier = m_currChar;
            // [A-z]([A-z]|[1-9])*
            while(std::isalnum(nextchar())){
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
            // if not a keyword, it's an identifier
            return Token::IDENTIFIER;
        }

        // Numbers
        if(std::isdigit(m_currChar) || m_currChar=='.'){
            std::string numStr;
            do{
                numStr += m_currChar;
                nextchar();
            } while (std::isdigit(m_currChar) || m_currChar=='.');

            m_numVal = std::stoi(numStr);
            return Token::NUMBER;
        }

        // Comments
        if(m_currChar=='#'){
            do{
                nextchar();
            } while (m_currChar != EOF && m_currChar != '\n' && m_currChar != '\r');

            if(m_currChar != EOF){
                return nextToken();
            }
        }

        if(m_currChar==EOF){
            return Token::ENDFILE;
        }

        nextchar();
        return Token::UNKNOWN;
    }

    //almost the same as processToken(), but also update m_currTok.
    inline Token nextToken(){
        return m_currTok = processToken();
    }
};

#endif // TESTLANG_LEXER_HPP