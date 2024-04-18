#include "operations.hpp"

Operation getOpFromKey(std::string t_key) {
    auto lookup = operations::opKeys.find(t_key);
    if (lookup == operations::opKeys.end()) {
        return {-1,nullptr};
    }
    return lookup->second;
}