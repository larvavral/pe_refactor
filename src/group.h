#ifndef GROUP_H_
#define GROUP_H_

#include <string>
#include <thread>
#include <vector>
#include "configuration.h"
#include "hiredis/adapters/libevent.h"
#include "redis_controller.h"
#include "symbol.h"

class Group {
public:
  Group();
  Group(const RedisServerInformation& redis_info,
        const GroupInformation& group_info,
        struct event_base* event_base);
  virtual ~Group();

  // Control generated price loop.
  void StartLoop();
  void StopLoop();

private:
  // Event functions of async connection.
  // Called after send command authenticate to redis server.
  void OnAsyncConnectAuthenticated(redisReply* reply);
  // Called when received notify from NOP that dealer updated configuration.
  // These configuration is set on NOP (not configuration read from file).
  void OnPEConfigUpdated(redisReply* reply);

  // Establish connection to redis server, and create |Symbol| objects.
  void Initialize(const RedisServerInformation& redis_info,
                  const GroupInformation& group_info,
                  struct event_base* event_base);

  // Read fair value config from redis server.
  bool GetPEConfigFromRedis(const std::string& symbol_name,
                            FairValueConfig& fair_value_config);

  // Send fair value data (fair value, moving average, bid, ask, etc.) to redis.
  void SendFairValueToRedis(const std::string& symbol_name,
                            double fair_value,
                            double moving_average,
                            double standard_deviation_ratio);

  // Loop to generate price of all symbols in this group.
  void Loop();

  // Setting variables.
  // std::string base_symbol_;
  // std::vector<std::string> price_sources_;
  // std::vector<std::string> symbol_;

  // Redis controller.
  redisContext* redis_client_;
  redisAsyncContext* async_connect_;

  // List all |Symbol| in this group.
  std::vector<std::unique_ptr<Symbol>> symbols_;

  // Other.
  // Mutex to protect get/set price process inside group.
  std::mutex price_mutex_;

  // This thread use to run |Loop()|.
  std::thread loop_thread_;

  // Use to stopping loop when necessary.
  bool stop_loop_ = false;
};

#endif  // GROUP_H_