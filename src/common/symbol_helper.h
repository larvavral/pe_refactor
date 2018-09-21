#ifndef COMMON_SYMBOL_HELPER_H_
#define COMMON_SYMBOL_HELPER_H_

#include <string>

namespace common {

// Get code of crypto/currency from symbol.
// |pos| is position of code in symbol, 1 mean first code, 2 mean second code.
std::string GetCodeFromSymbol(const std::string& symbol, int pos);

uint64_t GetCurrentTimestamp();

} // namespace common

#endif  // COMMON_SYMBOL_HELPER_H_