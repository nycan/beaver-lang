#ifndef TESTLANG_LEXER_HPP
#define TESTLANG_LEXER_HPP

#include <string>
#include <cctype>

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
    static std::string m_identifier;
    static double m_numVal;
    static Token m_currTok;
    static int m_currChar;

public:
    inline std::string& getIdentifier() const {return m_identifier;}
    inline double getNum() const {return m_numVal;}
    inline Token getTok() const {return m_currTok;}
    inline char getChar() const {return m_currChar;}

    //read the next token
    static Token processToken(){
        static int m_currChar = ' ';
        while(std::isspace(m_currChar)){
            m_currChar = getchar();
        }

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

        m_currChar = getchar();
        return Token::UNKNOWN;
    }

    //almost the same as nextToken(), but also update m_currTok.
    static Token nextToken(){
        return m_currTok = processToken();
    }
};

#endif