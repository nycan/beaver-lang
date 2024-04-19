#include "parser.hpp"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"

// return the index of an option, if it exists
size_t findOption(int argc, char **argv, const std::string& option) {
  auto result = std::find(argv, argv+argc, option);
  if (result) {
    return result-argv;
  }
  return 0;
}

int main(int argc, char **argv) {
  std::cout << "Initializing target\n";
  // get the string representing the system to compile to
  std::string targetTriple = llvm::sys::getDefaultTargetTriple();
  if (size_t argIndex = findOption(argc-2, argv, "-target")) {
    if (argv[argIndex+1][0] == '-') {
      llvm::errs() << "Expected target triple.";
      return 1;
    }
    targetTriple = argv[argIndex+1];
  }

  // initialize stuff
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  // get the target from the triple
  std::string error;
  auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);

  if (!target) {
    llvm::errs() << error;
    return 1;
  }

  // also use defaults for cpu and features
  auto CPU = "generic";
  auto features = "";

  // get the machine from the target
  llvm::TargetOptions options;
  auto targetMachine = target->createTargetMachine(targetTriple, CPU, features,
                                                   options, llvm::Reloc::PIC_);

  std::cout << "Initializing generator\n";
  // initialize the generator
  auto generator = std::make_shared<Generator>();
  generator->m_module.setDataLayout(targetMachine->createDataLayout());
  generator->m_module.setTargetTriple(targetTriple);

  std::cout << "Initializing output file\n";
  // get the output file
  std::string outputFile = "output.o";
  if (size_t argIndex = findOption(argc-2, argv, "-o")) {
    if (argv[argIndex+1][0] == '-') {
      llvm::errs() << "Expected output filename.";
      return 1;
    }
    outputFile = argv[argIndex+1];
  }

  // initialize the ouput stream
  std::error_code errorCode;
  llvm::raw_fd_ostream outputStream(outputFile, errorCode,
                                    llvm::sys::fs::OF_None);

  if (errorCode) {
    llvm::errs() << "Could not open file: " << errorCode.message();
    return 1;
  }

  std::cout << "Initializing lexer with file " << argv[argc-1] << '\n';
  // create the lexer and parser
  std::unique_ptr<Lexer> lex = std::make_unique<FileLexer>(argv[argc-1]);
  std::cout << "Initializing parser\n";
  Parser parse(std::move(lex), generator);

  std::cout << "Parsing and generating code\n";
  // parse and generate code
  if (parse()) {
    return 1;
  }

  std::cout << "Initializing pass manager\n";
  llvm::legacy::PassManager pass;
  llvm::CodeGenFileType outputType;
  if (findOption(argc, argv, "-S")) {
    outputType = llvm::CodeGenFileType::AssemblyFile;
  } else {
    outputType = llvm::CodeGenFileType::ObjectFile;
  }

  if (targetMachine->addPassesToEmitFile(pass, outputStream, nullptr,
                                         outputType)) {
    llvm::errs() << "Invalid file type";
    return 1;
  }

  std::cout << "Running pass manager\n";
  pass.run(generator->m_module);
  outputStream.flush();
}