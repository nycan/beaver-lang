#include "parser.hpp"

int main(){
    Lexer lex;
    Parser parse(lex,std::make_shared<Generator>());
    parse();
}