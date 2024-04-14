#include "parser.hpp"

// helper function for blocks
std::vector<std::unique_ptr<SyntaxTree>> Parser::parseBlock(){
    // parse '{'
    if(m_lexer.getChar() != '{'){
        llvm::errs() << "Expected '{'.";
        return std::vector<std::unique_ptr<SyntaxTree>>();
    }
    m_lexer.nextToken();

    // parse body
    std::vector<std::unique_ptr<SyntaxTree>> result;
    while(m_lexer.getChar() != '}'){
        if(m_lexer.getChar() == EOF){
            llvm::errs() << "Expected '}'.";
            return std::vector<std::unique_ptr<SyntaxTree>>();
        }
        if(auto line = parseMain()){
            result.push_back(line);
        } else {
            return std::vector<std::unique_ptr<SyntaxTree>>();
        }
    }

    //parse '}'
    m_lexer.nextToken();
    if(result.size() == 0){
        llvm::errs() << "Expected at least one line in definition.\n";
    }
    return result;
}

std::unique_ptr<SyntaxTree> Parser::parseNum(){
    auto result = std::make_unique<NumberAST>(m_genData, m_lexer.getNum());
    m_lexer.nextToken();
    return std::move(result);
}

std::unique_ptr<SyntaxTree> Parser::parseParens(){
    // parse '('
    m_lexer.nextToken();

    // parse inside expression
    auto exprResult = parseExpression();
    if (!exprResult){
        return nullptr;
    }

    // parse ')'
    if(m_lexer.getChar() != ')'){
        llvm::errs() << "Missing ')'\n";
        return nullptr;
    }
    m_lexer.nextToken();

    return exprResult;
}

std::unique_ptr<SyntaxTree> Parser::parseIdentifier(){
    // parse identifier
    std::string idName = m_lexer.getIdentifier();
    m_lexer.nextToken();

    // variable
    if(m_lexer.getChar() != '('){
        return std::make_unique<VariableAST>(m_genData, idName);
    }

    // function call
    m_lexer.nextToken();
    std::vector<std::unique_ptr<SyntaxTree>> args;
    if(m_lexer.getChar() != ')'){
        while(true){
            // parse argument
            if(auto argument = parseExpression()){
                args.push_back(std::move(argument));
            } else {
                return nullptr;
            }
            
            // end of argument list
            if(m_lexer.getChar() == ')'){
                break;
            }

            // separator
            if(m_lexer.getChar() != ','){
                llvm::errs() << "Expected ')' or ',' in argument list.";
                return nullptr;
            }
            m_lexer.nextToken();
        }
    }

    // parse ')'
    m_lexer.nextToken();
    return std::make_unique<CallAST>(m_genData, idName, std::move(args));
}

std::unique_ptr<SyntaxTree> Parser::parseConditional(){
    // parse 'if'
    m_lexer.nextToken();

    // parse condition
    auto condition = parseExpression();
    if(!condition){
        return nullptr;
    }

    // parse the "then"
    std::vector<std::unique_ptr<SyntaxTree>> mainBlock = parseBlock();
    if(mainBlock.size() == 0){ 
        return nullptr;
    }

    // if an else block exists, parse it
    // nullptr is used for a non-existant else block
    std::vector<std::unique_ptr<SyntaxTree>> elseBlock;
    if(m_lexer.getTok() == Token::ELSE){
        // parse "else"
        m_lexer.nextToken();

        elseBlock = parseBlock();
        if(elseBlock.size() == 0){
            return nullptr;
        }
    }

    return std::make_unique<ConditionalAST>(
        m_genData, std::move(condition),std::move(mainBlock),std::move(elseBlock)
    );
}

// helper function for parseMain to parse the last character when the token is unknown
std::unique_ptr<SyntaxTree> Parser::handleUnknown(){
    switch(m_lexer.getChar()){
        case '(':
            return parseParens();
        default:
            std::cerr << m_lexer.getChar() << '\n';
            llvm::errs() << "Unknown token\n";
            return nullptr;
    }
}

// main parse function
std::unique_ptr<SyntaxTree> Parser::parseMain(){
    switch(m_lexer.getTok()){
        case Token::IDENTIFIER:
            return parseIdentifier();
        case Token::NUMBER:
            return parseNum();
        case Token::IF:
            return parseConditional();
        case Token::RETURN:
            return parseReturn();
        default:
            return handleUnknown();
    }
}

// assigns precedence to operations
int Parser::BinopPrecedence(){ //TODO: replace this with something better
    switch(m_lexer.getChar()){
        case '*':
            return 3;
        case '/':
            return 3;
        case '+':
            return 2;
        case '-':
            return 2;
        case '<':
            return 1;
        case '>':
            return 1;
        default:
            return -1;
    }
}

std::unique_ptr<SyntaxTree> Parser::parseOpRHS(
    const int t_minPrec,
    std::unique_ptr<SyntaxTree> t_leftSide
){
    while(true){
        int precedence = BinopPrecedence();

        // if it is lower precedence, then it will be parsed in a different call
        // since unknown operators return -1, this handles the no-operation case
        if(precedence < t_minPrec){
            return t_leftSide;
        }

        // parse operation
        char operation = m_lexer.getChar();
        m_lexer.nextToken();

        // parse right side
        auto rightSide = parseMain();
        if(!rightSide){
            return nullptr;
        }

        // if the expression continues, parse it
        int nextPrec = BinopPrecedence();
        // if the next operator is higher precedence, it needs to be handled before this one
        // parse recursively
        if(precedence < nextPrec){
            rightSide = parseOpRHS(precedence+1, std::move(rightSide));
            if(!rightSide){
                return nullptr;
            }
        }

        t_leftSide = std::make_unique<BinaryOpAST>(
            m_genData,
            operation,
            std::move(t_leftSide),
            std::move(rightSide)
        );
    }
}

std::unique_ptr<SyntaxTree> Parser::parseExpression(){
    auto leftSide = parseMain();
    if(!leftSide){
        return nullptr;
    }
    return parseOpRHS(0, std::move(leftSide));
}

std::unique_ptr<PrototypeAST> Parser::parsePrototype(){
    if(m_lexer.getTok() != Token::IDENTIFIER){
        llvm::errs() << "Expected function name in prototype.\n";
        return nullptr;
    }
    
    //function name
    std::string funcName = m_lexer.getIdentifier();
    m_lexer.nextToken();

    //arguments
    if(m_lexer.getChar() != '('){
        llvm::errs() << "Expected '('\n";
        return nullptr;
    }
    m_lexer.nextToken();
    std::vector<std::string> args;
    if(m_lexer.getChar() != ')'){
        while(true){
            // parse argument
            if(m_lexer.getTok() != Token::IDENTIFIER){
                llvm::errs() << "Unexpected token in prototype\n";
                return nullptr;
            }
            args.push_back(m_lexer.getIdentifier());
            m_lexer.nextToken();
            
            // end of argument list
            if(m_lexer.getChar() == ')'){
                break;
            }

            // separator
            if(m_lexer.getChar() != ','){
                llvm::errs() << "Expected ')' or ',' in argument list.";
                return nullptr;
            }
            m_lexer.nextToken();
        }
    }

    // parse ')'
    if(m_lexer.getChar() != ')'){
        llvm::errs() << "Expected ')'\n";
        return nullptr;
    }
    m_lexer.nextToken();

    return std::make_unique<PrototypeAST>(m_genData, funcName, std::move(args));
}

std::unique_ptr<SyntaxTree> Parser::parseReturn(){
    // parse "ret"
    m_lexer.nextToken();

    if(auto resAST = parseMain()){
        return std::make_unique<ReturnAST>(m_genData,std::move(resAST));
    }
    return nullptr;
}

std::unique_ptr<FunctionAST> Parser::parseDefinition(){
    // function declaration
    m_lexer.nextToken();

    // prototype
    auto prototype = parsePrototype();
    if(!prototype){
        return nullptr;
    }

    // body
    if(m_lexer.getChar() != '{'){
        llvm::errs() << "Expected definition.";
        return nullptr;
    }

    std::vector<std::unique_ptr<SyntaxTree>> block = parseBlock();
    if(block.size() > 0){
        return std::make_unique<FunctionAST>(m_genData, std::move(prototype), std::move(block));
    }

    return nullptr;
}

std::unique_ptr<PrototypeAST> Parser::parseExtern(){
    m_lexer.nextToken();
    return parsePrototype();
}

// wrap top-level expressions in an anonymous prototype
std::unique_ptr<FunctionAST> Parser::parseTopLevel(){
    if(auto expr = parseExpression()){
        auto prototype = std::make_unique<PrototypeAST>(
            m_genData,
            "somethingThatIllProbablyForgetToChange", // Don't forget to change this
            std::vector<std::string>()
        );
        return std::make_unique<FunctionAST>(m_genData, std::move(prototype),std::move(expr));
    }
    return nullptr;
}

//temporary main function
bool Parser::operator()(){
    m_lexer.nextToken();
    bool hasErrors = false;

    while(true){
        switch(m_lexer.getTok()){
            case Token::ENDFILE:
                return;
            case Token::FUNC:
                if(auto resAST = parseDefinition()){
                    if(auto* resIR = resAST->codegen()){
                        std::cerr << "Successfully parsed function definition.\n";
                    } else {
                        hasErrors = true;
                    }
                } else {
                    hasErrors = true;
                    m_lexer.nextToken();
                }
                break;
            case Token::EXTERN:
                if(auto resAST = parseExtern()){
                    if(auto* resIR = resAST->codegen()){
                        std::cerr << "Successfully parsed extern.\n";
                    } else {
                        hasErrors = true;
                    }
                } else {
                    hasErrors = true;
                    m_lexer.nextToken();
                }
                break;
            default:
                if(m_lexer.getChar()==';'){
                    m_lexer.nextToken();
                } else {
                    if(auto resAST = parseTopLevel()){
                        if(auto* resIR = resAST->codegen()){
                            std::cerr << "Successfully parsed top-level expression.\n";
                            resIR->removeFromParent();
                        } else {
                            hasErrors = true;
                        }
                    } else {
                        hasErrors = true;
                        m_lexer.nextToken();
                    }
                }
                break;
        }
    }
    return hasErrors;
}