#include "operations.hpp"

std::optional<Operation> getBinOp(std::string t_key) {
  auto lookup = operations::opKeys.find(t_key);
  if (lookup == operations::opKeys.end()) {
    return {};
  }
  return lookup->second;
}

std::optional<Operation> getAssignmentOp(std::string t_key) {
  auto lookup = operations::assignmentKeys.find(t_key);
  if (lookup == operations::assignmentKeys.end()) {
    return {};
  }
  return lookup->second;
}