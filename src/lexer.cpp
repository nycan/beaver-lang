#include "lexer.hpp"

char Lexer::nextChar(){
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

Token Lexer::processToken(){
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