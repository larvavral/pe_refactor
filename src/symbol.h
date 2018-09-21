#ifndef SYMBOL_H_
#define SYMBOL_H_

#include <mutex>
#include <queue>
#include <string>
#include <vector>
#include "redis_controller.h"

// Define the way to calculate fair value of a symbol.
enum CalculateFairValueMethod {
  // Fair value is calculated based on price (order book) from
  // other trading platform.
  FROM_OTHER_SOURCES = 0,
  // Fixed price (usually use in case cannot get price from other trading
  // platform, or starting ICO crypto).
  FIXED_PRICE,
  // Fair value is calculated based on a currency.
  // Ex:
  //    fair value of BTCETH = BTCJPY / ETHJPY
  //    (in case of we use JPY as base currency)
  BASED_ON_A_CURRENCY,
  // Just use to know how many way to calculate fair value.
  CALCULATE_METHOD_MAX
};

// Type to get price from order books of other sources.
enum SourceType {
  BID = 1,
  ASK,
  MID,
  SOURCE_TYPE_MAX
};

struct FairValueConfig {
  // Way to calculate fair value.
  CalculateFairValueMethod calculate_method;

  // Use when calculate fair value based on other sources.
  // How many percentage a source affects to fair value.
  std::vector<double> source_percentage;
  // Type to get price from order books of other sources.
  std::vector<SourceType> source_type;
  // Use to filter source's price.
  double filter_ratio;
  // Ignore order book with position is less then |lot_limit| value.
  // These price is too mobility and is not good enough for
  // calculating fair value.
  int lot_limit;

  // Use when calculate fair value based on fixed price.
  double fixed_price;

  // Use when calculate fair value based on a currency.
  // Currently, just use "jpy" (NOP did not send this setting).
  std::string base_currency = "jpy";

  // Skew settings. Use to update fair value after calculated.
  // Skew feature is on or off.
  bool skew_active;
  // Skew type. There are 2 type of skewing: value and percent.
  int skew_type;
  // Skew value.
  double skew_value;
  // Skew percent (%).
  double skew_percent;

  // Others.
  int moving_average;
};

// Contain all methods of a symbol in Price Engine,
// include: calculate fair value, etc.
class Symbol {
public:
  Symbol(const std::string& symbol_name,
         const FairValueConfig& config,
         redisContext* redis_client);
  // Symbol(const Symbol& other) = default;
  // Symbol& operator=(const Symbol& other) = default;
  virtual ~Symbol();

  // Update settings which are used to calculate fair value.
  // This is update each time receive notify from NOP.
  void UpdateFairValueConfig(FairValueConfig config);

  // Calculate fair value of this symbol.
  double CalculateFairValue();

  // Other value need to be calculated, include: moving average,
  void CalculateMovingAverage(double& moving_average,
                              double& standard_deviation,
                              double& standard_deviation_ratio);

  std::string GetSymbolName() { return symbol_name_; }

private:
  // Get fair value of base currency from redis.
  // Used to calculate fair value by |BASED_ON_A_CURRENCY| method.
  double GetBaseCurrencyFairValue(const std::string& symbol);

  // Symbol name.
  std::string symbol_name_;

  // Settings, used to calculate fair value.
  FairValueConfig fair_value_config_;

  // Save old fair value to calculate moving average.
  std::deque<double> fair_value_history_;

  // Redis client, used to get data from redis.
  redisContext* redis_client_;

  // Mutexes to protect behavior of symbol class.
  std::mutex setting_mutex_;
};

#endif  // SYMBOL_H_