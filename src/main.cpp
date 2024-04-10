#include "parser.hpp"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"

int main(){
    // for now, just check the target automatically
    std::string targetTriple = llvm::sys::getDefaultTargetTriple();

    // initialize stuff
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    // get the target from the triple 
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple,error);

    if(!target){
        llvm::errs() << error;
        return 1;
    }

    // also use defaults for cpu and features
    auto CPU = "generic";
    auto features = "";

    // get the machine from the target
    llvm::TargetOptions options;
    auto targetMachine = target->createTargetMachine(
        targetTriple, CPU, features, options, llvm::Reloc::PIC_
    );

    // initialize the generator
    auto generator = std::make_shared<Generator>();
    generator->m_module->setDataLayout(targetMachine->createDataLayout());
    generator->m_module->setTargetTriple(targetTriple);

    // initialize the output stream
    auto outputFile = "output.o";
    std::error_code errorCode;
    llvm::raw_fd_ostream outputStream(outputFile, errorCode, llvm::sys::fs::OF_None);

    if(errorCode){
        llvm::errs() << "Could not open file: " << errorCode.message();
        return 1;
    }

    // create the lexer and parser
    FileLexer lex("test.txt");
    Parser parse(lex,generator);

    // parse and generate code
    if(parse()){
        return 1;
    }

    llvm::legacy::PassManager pass;
    auto fileType = llvm::CodeGenFileType::ObjectFile;

    if(targetMachine->addPassesToEmitFile(pass, outputStream, nullptr, fileType)){
        llvm::errs() << "Invalid file type";
        return 1;
    }

    pass.run(*generator->m_module);
    outputStream.flush();
}