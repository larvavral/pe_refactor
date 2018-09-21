#include "common/symbol_helper.h"

#include "glog/logging.h"

namespace common {

std::string GetCodeFromSymbol(const std::string& symbol, int pos) {
  DCHECK(symbol.size() == 6);
  DCHECK(pos == 1 || pos == 2);
  return pos == 1 ? symbol.substr(0, 3) : symbol.substr(3, 3);
}

uint64_t GetCurrentTimestamp() {
  using std::chrono::system_clock;
  return static_cast<uint64_t>(system_clock::to_time_t(system_clock::now()));
}

} // namespace common