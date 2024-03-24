#include "parser.hpp"

int main(){
    Lexer lex;
    Parser parse(lex);
    parse();
}