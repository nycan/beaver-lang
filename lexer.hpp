#ifndef TESTLANG_LEXER_HPP
#define TESTLANG_LEXER_HPP

#include <string>
#include <cctype>

class Lexer{
private:
    static std::string m_identifier;
    static double m_numVal;
    enum Token{
        eofToken = -1,
        funcToken = -2,
        externToken = -3,
        identifierToken = -4,
        numberToken = -5
    };
public:
    static int nextToken(){
        static int lastChar = ' ';
        while(std::isspace(lastChar)){
            lastChar = getchar();

            // Keywords and identifiers
            if(std::isalpha(lastChar)){
                m_identifier = lastChar;
                while(std::isalnum(lastChar=getchar())){
                    m_identifier += lastChar;
                }

                if(m_identifier=="fn"){
                    return funcToken;
                }
                if(m_identifier=="extern"){
                    return externToken;
                }
                return identifierToken;
            }

            // Numbers
            if(std::isdigit(lastChar) || lastChar=='.'){
                std::string numStr;
                do{
                    numStr += lastChar;
                    lastChar = getchar();
                } while (std::isdigit(lastChar) || lastChar=='.');

                m_numVal = std::stoi(numStr);
                return numberToken;
            }

            // Comments
            if(lastChar=='#'){
                do{
                    lastChar = getchar();
                } while (lastChar != EOF && lastChar != '\n' && lastChar != '\r');

                if(lastChar != EOF){
                    return nextToken();
                }
            }

            if(lastChar==EOF){
                return eofToken;
            }

            int currChar = lastChar;
            lastChar = getchar();
            return currChar;
        }
    }
};

#endif