#include "operations.hpp"

std::optional<Operation> getOpFromKey(std::string t_key) {
  auto lookup = operations::opKeys.find(t_key);
  if (lookup == operations::opKeys.end()) {
    return {};
  }
  return lookup->second;
}