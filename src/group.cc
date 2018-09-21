#include "group.h"

#include "common/symbol_helper.h"
#include "glog/logging.h"
#include "nlohmann/json.hpp"
#include "redis_key.h"

namespace {

// Time to reconnect to redis after fail connection. (milliseconds)
const int kReconnectTime = 1000;

} // namespace

Group::Group() {
}

Group::Group(const RedisServerInformation& redis_info,
             const GroupInformation& group,
             struct event_base* event_base) {
  Initialize(redis_info, group, event_base);
}

Group::~Group() {
}

void Group::Initialize(
    const RedisServerInformation& redis_info,
    const GroupInformation& group,
    struct event_base* event_base) {
  // Connect to redis server.
  redis_client_ =
      redis::client::CreateRedisClient(redis_info.host, redis_info.port);
  if (redis_client_ == nullptr ||
      !redis::client::Authenticate(redis_client_, redis_info.password)) {
    // TODO(hoangpq): Setup a timer to reconnect. (Re-initialize)
    // Try to do not exit everytime cannot establish connection to redis.

    // std::thread([&]() {
    //   std::this_thread::sleep_for(std::chrono::milliseconds(kReconnectTime));
    //   // Initialize(redis_info, group, event_base);
    // }).detach();

    // Plz change to LOG(ERROR) if you don't want to receive failure stack to
    // log file when authenticate failed.
    LOG(FATAL) << "Cannot establish connection to redis! Exit.";
    std::exit(EXIT_FAILURE);
    return;
  }

  // Create async connect to redis server.
  async_connect_ = redis::async_connect::CreateAsyncConnect(
      redis_info.host, redis_info.port);
  if (async_connect_ == nullptr)
    return;

  // Attach the redisAsyncContext to event_base of libevent.
  redisLibeventAttach(async_connect_, event_base);

  // Register all callbacks for async connection here, before
  // |event_base_dispatch()| function is called on main().
  using namespace std::placeholders;
  redis::async_connect::Authenticate(
      async_connect_, redis_info.password,
      std::bind(&Group::OnAsyncConnectAuthenticated, this, _1));

  redis::async_connect::Subscribe(
      async_connect_, kPEConfigChannel,
      std::bind(&Group::OnPEConfigUpdated, this, _1));

  // Create |Symbol| objects correspond with symbol list in the setting file.
  for (auto& symbol_name : group.symbols) {
    // Get fair value configuration at the first time.
    FairValueConfig fair_value_config;
    if (!GetPEConfigFromRedis(symbol_name, fair_value_config)) {
      LOG(ERROR) << "Cannot get PE config for symbol " << symbol_name
                 << ". Ignore this symbol, please restart application.";
      continue;
    }

    // Save instances of |Symbol| into a vector to refer later.
    std::unique_ptr<Symbol> symbol(
        new Symbol(symbol_name, fair_value_config, redis_client_));
    symbols_.push_back(std::move(symbol));
  }
}

void Group::StartLoop() {
  stop_loop_ = false;

  // Start loop to generate price of symbols.
  loop_thread_ = std::thread(&Group::Loop, this);
}

void Group::StopLoop() {
  stop_loop_ = true;
  loop_thread_.join();
}

void Group::OnAsyncConnectAuthenticated(redisReply* reply) {
  LOG(INFO) << "OnAsyncConnectAuthenticated()";

  if (reply->type == REDIS_REPLY_ERROR)
    LOG(ERROR) << "Async connection authenticated fail!";
  else
    LOG(INFO) << "Async connection authenticated successfully.";
}

void Group::OnPEConfigUpdated(redisReply* reply) {
  LOG(INFO) << "OnPEConfigUpdated()";

  // Small check to be sure that data is valid.
  if (reply == nullptr ||
      reply->type != REDIS_REPLY_ARRAY ||
      reply->elements != 3)
    return;

  // In some cases, message can be NULL. So a judgment is necessary.
  if (reply->element[2]->str == nullptr)
    return;
  std::string message = std::string(reply->element[2]->str);

  // This callback is notified to all symbols eventhough we just update
  // a symbol, so we just update which symbol is named in message.
  for (auto& symbol : symbols_) {
    if (message == symbol->GetSymbolName()) {
      FairValueConfig fair_value_config;
      if (GetPEConfigFromRedis(message, fair_value_config))
        symbol->UpdateFairValueConfig(fair_value_config);

      break;
    }
  }
}

bool Group::GetPEConfigFromRedis(const std::string& symbol_name,
                                 FairValueConfig& fair_value_config) {
  // Read value of PE configuration key from redis.
  std::string message =
      redis::client::Get(redis_client_,
          std::string(kPEConfigPrefix) + symbol_name);
  if (message.empty())
    return false;

  // Extract setting values from json value.
  nlohmann::json json = nlohmann::json::parse(message);
  try {
    // TODO(hoangpq): Currently, calculate_method and fixed_price send from NOP
    // are string. Plz change to number for easy handling.
    // TODO(hoangpq): Consider to use json::value() with default value instead
    // of json::get().
    try {
      std::string value = json[kCalculationMethodKey].get<std::string>();
      fair_value_config.calculate_method =
          static_cast<CalculateFairValueMethod>(std::stoi(value));
    } catch(std::exception e) {
      fair_value_config.calculate_method =
          static_cast<CalculateFairValueMethod>(json[kCalculationMethodKey].get<int>());
    }
    // TODO(hoangpq): Currently, we do not implememted calculation method 1,
    // so ignore |source_percentage| and |source_type| fields.
    // fair_value_config.calculate_method =
    //     static_cast<CalculateFairValueMethod>(
    //         json[kCalculationMethodKey].get<int>());

    fair_value_config.lot_limit = json[kLotLimitKey].get<int>();
    fair_value_config.filter_ratio = json[kFilterRatioKey].get<double>();

    try {
      std::string value = json[kFixedPrice].get<std::string>();
      fair_value_config.fixed_price = std::stod(value);
    } catch (std::exception e) {
      fair_value_config.fixed_price = json[kFixedPrice].get<double>();;
    }

    // Skew is nested object inside |json|.
    nlohmann::json skew = json[kSkewKey];
    fair_value_config.skew_active = skew[kSkewActiveKey].get<int>();
    fair_value_config.skew_type = skew[kSkewTypeKey].get<int>();
    fair_value_config.skew_value = skew[kSkewValueKey].get<double>();
    fair_value_config.skew_percent = skew[kSkewPercentKey].get<double>();

    fair_value_config.moving_average = json[kMovingAverageKey].get<int>();
  } catch (std::exception e) {
    LOG(ERROR) << "Error while reading PE config from redis: \n"
               << "message = " << json;
    return false;
  }

  return true;
}

void Group::SendFairValueToRedis(
    const std::string& symbol_name,
    double fair_value,
    double moving_average,
    double standard_deviation_ratio) {
  nlohmann::json json;

  // Set data to json.
  uint64_t now = common::GetCurrentTimestamp();
  json[kTimestampKey] = std::to_string(now);
  json[kFairValueKey] = std::to_string(fair_value);
  json[kFairValueMVKey] = std::to_string(moving_average);
  json[kStdDevRatioKey] = std::to_string(standard_deviation_ratio);

  // Send data to redis.
  redis::client::Set(redis_client_,
      std::string(kFairValuePrefix) + symbol_name,
      json.dump());

  redis::async_connect::Publish(async_connect_,
      std::string(kFairValueChannel),
      symbol_name,
      nullptr);
}

void Group::Loop() {
  uint64_t loop_interval = Configuration::GetInstance()->GetLoopInterval();

  // Each |loop_interval| milliseconds, loop run and generate price for all
  // symbols.
  while (!stop_loop_) {
    uint64_t start_time = common::GetCurrentTimestamp();

    for (auto& symbol : symbols_) {
      double fv = symbol->CalculateFairValue();
      if (fv <= 0)
        continue;

      double mv = 0.0;
      double std_dev = 0.0;
      double std_dev_ratio = 0.0;
      symbol->CalculateMovingAverage(mv, std_dev, std_dev_ratio);

      // Logging
      LOG(INFO) << "Generated fair value for [" << symbol->GetSymbolName()
                << "]: fair_value = " << fv << ", "
                << "moving_average = " << mv << ", "
                << "std_dev = " << std_dev << ", "
                << "std_dev_ratio = " << std_dev_ratio;

      // Send data to redis.
      SendFairValueToRedis(symbol->GetSymbolName(),
                           fv, mv, std_dev_ratio);
    }

    // Use these values to analyze performance when necessary.
    uint64_t stop_time = common::GetCurrentTimestamp();
    uint64_t execution_time = stop_time - start_time;
    uint64_t sleep_time =
        (execution_time >= loop_interval) ? 0 : (loop_interval - execution_time);

    // Wait until next loop.
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
  }
}