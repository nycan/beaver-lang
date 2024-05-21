#include "parser.hpp"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"

// return the index of an flag, if it exists
size_t findOption(int argc, char **argv, const std::string &option) {
  if (argc < 0) {
    return 0;
  }
  auto result = std::find(argv, argv + argc, option);
  if (result != argv+argc) {
    return result - argv;
  }
  return 0;
}

int main(int argc, char **argv) {
  // get the string representing the system to compile to
  std::string targetTriple = llvm::sys::getDefaultTargetTriple();

  // if a user-defined target triple exists, use that instead
  if (size_t argIndex = findOption(argc - 2, argv, "-target")) {
    if (argv[argIndex + 1][0] == '-') {
      llvm::errs() << "Expected target triple.\n";
      return 1;
    }
    targetTriple = argv[argIndex + 1];
  }

  // initialize stuff
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  // get the target class from the triple string
  std::string error;
  auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);

  if (!target) {
    llvm::errs() << error << '\n';
    return 1;
  }

  // Use defaults for cpu and features
  auto CPU = "generic";
  auto features = "";

  // Initialize the target machine with the target, CPU and features
  llvm::TargetOptions options;
  auto targetMachine = target->createTargetMachine(targetTriple, CPU, features,
                                                   options, llvm::Reloc::PIC_);

  // initialize the generator
  // Add the data layout and target triple here,
  // so that we don't need to pass it to the constructor
  auto generator = std::make_shared<Generator>();
  generator->m_module.setDataLayout(targetMachine->createDataLayout());
  generator->m_module.setTargetTriple(targetTriple);

  // Default output file: "output.o"
  // NOTE: this is not used right now
  std::string outputFile = "output.o";

  // If a user-defined output file exists, use it
  if (size_t argIndex = findOption(argc - 2, argv, "-o")) {
    if (argv[argIndex + 1][0] == '-') {
      llvm::errs() << "Expected output filename.\n";
      return 1;
    }
    outputFile = argv[argIndex + 1];
  }

  // initialize the ouput stream
  std::error_code errorCode;
  llvm::raw_fd_ostream outputStream(outputFile, errorCode,
                                    llvm::sys::fs::OF_None);

  if (errorCode) {
    llvm::errs() << "Could not open file: " << errorCode.message() << '\n';
    return 1;
  }

  // create the lexer and parser
  // last command line argument is the input file
  if (argc < 2) {
    llvm::errs() << "Expected input file.\n";
  }
  std::unique_ptr<Lexer> lex = std::make_unique<FileLexer>(argv[argc - 1]);
  Parser parse(std::move(lex), generator);

  // parse and generate code
  ParserStatus ps = parse.parseOuter();
  while (ps != ParserStatus::end) {
    ps = parse.parseOuter();
    if (ps == ParserStatus::error) {
      return 1;
    }
  }

  // Build the JIT engine
  auto engine = llvm::EngineBuilder(
    std::unique_ptr<llvm::Module>(&(generator->m_module))
  )
  .setErrorStr(&error)
  .setOptLevel(llvm::CodeGenOptLevel::Default)
  .setEngineKind(llvm::EngineKind::JIT)
  .create();

  if (!engine) {
    llvm::errs() << error << '\n';
    return 1;
  }
  
  // Finalize the engine and call the entry point function
  engine->finalizeObject();
  std::function<double()> entryPoint = reinterpret_cast<double(*)()>(
    engine->getPointerToNamedFunction("main")
  );
  std::cout << entryPoint() << '\n';

  /*
  All this code is not used right now, since a JIT is being used in place of a static compiler
  llvm::legacy::PassManager pass;
  llvm::CodeGenFileType outputType;
  if (findOption(argc, argv, "-S")) {
    outputType = llvm::CodeGenFileType::AssemblyFile;
  } else {
    outputType = llvm::CodeGenFileType::ObjectFile;
  }

  if (targetMachine->addPassesToEmitFile(pass, outputStream, nullptr,
                                         outputType)) {
    llvm::errs() << "Invalid file type\n";
    return 1;
  }

  pass.run(generator->m_module);
  outputStream.flush();
  */
}