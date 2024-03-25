#ifndef TESTLANG_LEXER_HPP
#define TESTLANG_LEXER_HPP

#include <string>
#include <cctype>
#include <iostream>

enum class Token{
    UNKNOWN,
    ENDFILE,
    FUNC,
    EXTERN,
    IDENTIFIER,
    NUMBER
};

class Lexer{
private:
    std::string m_identifier;
    double m_numVal;
    Token m_currTok;
    char m_currChar;

public:
    Lexer(): m_identifier(""), m_numVal(0), m_currTok(Token::UNKNOWN), m_currChar(' ') {}

    inline std::string getIdentifier() const {return m_identifier;}
    double getNum() const {return m_numVal;}
    Token getTok() const {return m_currTok;}
    char getChar() const {return m_currChar;}

    //read the next token
    Token processToken(){
        while(std::isspace(m_currChar)){
            m_currChar = getchar();
        }
        std::cerr << "'" << m_currChar << "'" << '\n';

        // Keywords and identifiers
        if(std::isalpha(m_currChar)){
            m_identifier = m_currChar;
            while(std::isalnum(m_currChar=getchar())){
                m_identifier += m_currChar;
            }

            if(m_identifier=="fn"){
                return Token::FUNC;
            }
            if(m_identifier=="extern"){
                return Token::EXTERN;
            }
            return Token::IDENTIFIER;
        }

        // Numbers
        if(std::isdigit(m_currChar) || m_currChar=='.'){
            std::string numStr;
            do{
                numStr += m_currChar;
                m_currChar = getchar();
            } while (std::isdigit(m_currChar) || m_currChar=='.');

            m_numVal = std::stoi(numStr);
            return Token::NUMBER;
        }

        // Comments
        if(m_currChar=='#'){
            do{
                m_currChar = getchar();
            } while (m_currChar != EOF && m_currChar != '\n' && m_currChar != '\r');

            if(m_currChar != EOF){
                return nextToken();
            }
        }

        if(m_currChar==EOF){
            return Token::ENDFILE;
        }

        int tmpChar = getchar();
        return Token::UNKNOWN;
    }

    //almost the same as nextToken(), but also update m_currTok.
    Token nextToken(){
        return m_currTok = processToken();
    }
};

#endif