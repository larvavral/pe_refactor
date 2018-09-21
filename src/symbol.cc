#include "symbol.h"

#include <algorithm>
#include <mutex>
// #include <chrono>
// #include <ctime>
#include "common/symbol_helper.h"
#include "configuration.h"
#include "glog/logging.h"
#include "nlohmann/json.hpp"
#include "redis_key.h"

namespace {

// Max size of fair value history queue.
const int kMaxSizeFairValueHistoryQueue = 600;

} // namespace

Symbol::Symbol(
    const std::string& symbol_name,
    const FairValueConfig& config,
    redisContext* redis_client)
    : symbol_name_(symbol_name),
      fair_value_config_(config),
      redis_client_(redis_client),
      setting_mutex_() {
}

Symbol::~Symbol() {
}

void Symbol::UpdateFairValueConfig(FairValueConfig config) {
  std::lock_guard<std::mutex> lock(setting_mutex_);
  fair_value_config_ = config;
}

double Symbol::CalculateFairValue() {
  std::lock_guard<std::mutex> lock(setting_mutex_);
  double fair_value = 0.0;
  std::string symbol;
  double p = 0.0;

  // Firstly, get fair value by calculation method.
  switch (fair_value_config_.calculate_method) {
  case FROM_OTHER_SOURCES:
    // TODO(hoangpq): Currently do not use this method.
    return 0.0;
    break;
  case FIXED_PRICE:
    fair_value = fair_value_config_.fixed_price;
    break;
  case BASED_ON_A_CURRENCY:
    // Base symbol is X-Y, get fair value of XY by the following formula:
    //    X-Y = X-'JPY' / Y-'JPY".

    // Get fair value of first code with base currency.
    symbol = common::GetCodeFromSymbol(symbol_name_, 1)
                + fair_value_config_.base_currency;
    p = GetBaseCurrencyFairValue(symbol);
    if (p == 0.0) {
      LOG(ERROR) << "Cannot get fair value of symbol " << symbol;
      return 0.0;
    }
    fair_value = p;

    // Get fair value of second code with base currency.
    symbol = common::GetCodeFromSymbol(symbol_name_, 2)
                + fair_value_config_.base_currency;
    p = GetBaseCurrencyFairValue(symbol);
    if (p == 0.0) {
      LOG(ERROR) << "Cannot get fair value of symbol " << symbol;
      return 0.0;
    }
    fair_value /= p;
    break;
  default:
    LOG(ERROR) << "Do not support this calculation method: "
               << static_cast<int>(fair_value_config_.calculate_method);
    return 0.0;
  }

  // Skewing if necessary.
  if (fair_value_config_.skew_active) {
    switch (fair_value_config_.skew_type) {
    case 1:
      // by value.
      fair_value += fair_value_config_.skew_value;
      break;
    case 2:
      // by percent.
      fair_value *= (fair_value_config_.skew_percent / 100);
      break;
    default:
      LOG(ERROR) << "Do not support this skewing type: "
                 << static_cast<int>(fair_value_config_.skew_type);
      break;
    }
  }

  // Save current fair value in queue to calculate moving average.
  if (fair_value > 0) {
    fair_value_history_.push_front(fair_value);
    if (fair_value_history_.size() > kMaxSizeFairValueHistoryQueue)
      fair_value_history_.pop_back();
  }

  return fair_value;
}

void Symbol::CalculateMovingAverage(
    double& moving_average,
    double& standard_deviation,
    double& standard_deviation_ratio) {
  if (fair_value_history_.size() == 0 ||
      fair_value_config_.moving_average == 0)
    return;

  // Mean.
  int num = std::min(static_cast<int>(fair_value_history_.size()),
                     fair_value_config_.moving_average);
  double sum = std::accumulate(fair_value_history_.begin(),
                               fair_value_history_.begin() + num,
                               0.0);
  moving_average = sum / num;

  // Standard deviation.
  double sq_sum = 0.0;
  for (auto it = fair_value_history_.begin();
       it != fair_value_history_.begin() + num;
       it++) {
    sq_sum = ((*it) - moving_average) * ((*it) - moving_average);
  }
  standard_deviation = std::sqrt(sq_sum / num);

  // Standard deviation ratio.
  standard_deviation_ratio =
      (moving_average > 1)
      ? (standard_deviation * 100 / moving_average)
      : 0.0;
}

double Symbol::GetBaseCurrencyFairValue(const std::string& symbol) {
  std::string message =
      redis::client::Get(redis_client_, std::string(kFairValuePrefix) + symbol);
  if (!message.empty()) {
    // Convert message to json.
    nlohmann::json json = nlohmann::json::parse(message);

    try {
      // Check timestamp.
      std::string value = json[kTimestampKey].get<std::string>();
      uint64_t timestamp = std::stoull(value);
      uint64_t now = common::GetCurrentTimestamp();
      if (now > timestamp + Configuration::GetInstance()->GetDiffTimeMax()) {
        LOG(INFO) << "Base fair value is too old, ignore it.";
        return 0.0;
      }

      // Convert fair value string to double.
      value = json[kFairValueMVKey].get<std::string>();
      return std::stod(value);
    } catch(std::exception& e) {
      LOG(ERROR) << "Error while getting base fair value";
    }
  }

  return 0.0;
}