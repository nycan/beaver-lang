#ifndef TESTLANG_OPERATIONS_H
#define TESTLANG_OPERATIONS_H

#include "llvm/IR/IRBuilder.h"
#include <functional>
#include <map>
#include <string>

struct Operation {
  int precedence;
  std::function<llvm::Value*(llvm::IRBuilder<>&)> codegen;
};

namespace operations {
  const std::map<std::string,Operation> opMap;
}

#endif // TESTLANG_OPERATIONS_H