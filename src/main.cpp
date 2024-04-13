#include "parser.hpp"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"

int main(){

    auto targetTriple = llvm::sys::getDefaultTargetTriple();

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple,error);

    if(!target){
        llvm::errs() << error;
        return 1;
    }

    auto CPU = "generic";
    auto features = "";

    llvm::TargetOptions options;
    auto targetMachine = target->createTargetMachine(
        targetTriple, CPU, features, options, llvm::Reloc::PIC_
    );

    auto generator = std::make_shared<Generator>();
    generator->m_module->setDataLayout(targetMachine->createDataLayout());
    generator->m_module->setTargetTriple(targetTriple);

    auto outputFile = "output.o";
    std::error_code errorCode;
    llvm::raw_fd_ostream outputStream(outputFile, errorCode, llvm::sys::fs::OF_None);

    if(errorCode){
        llvm::errs() << "Could not open file: " << errorCode.message();
        return 1;
    }

    Lexer lex;
    Parser parse(lex,generator);
    parse();

    llvm::legacy::PassManager pass;
    auto fileType = llvm::CodeGenFileType::ObjectFile;  // Change this to llvm::CodeGenFileType::AssemblyFile to compile into assembly 

    if(targetMachine->addPassesToEmitFile(pass, outputStream, nullptr, fileType)){
        llvm::errs() << "Invalid file type";
        return 1;
    }

    pass.run(*generator->m_module);
    outputStream.flush();
}
