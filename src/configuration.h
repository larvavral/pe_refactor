#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <string>
#include <vector>

struct RedisServerInformation {
  std::string host;
  int port;
  std::string password;
};

struct GroupInformation {
  std::string base_symbol;
  std::vector<std::string> price_sources;
  std::vector<std::string> symbols;
};

// This class contains all settings of this application.
// It's a singleton.
class Configuration {
public:
  static Configuration* GetInstance();

  // Delete these 2 functions to make sure not get copies of this
  // singleton accidentally.
  Configuration(Configuration const&) = delete;
  void operator=(Configuration const&) = delete;

  // Load all settings from config file.
  void LoadConfig(const std::string& file_name);

  // Get settings.
  RedisServerInformation GetRedisServerInfo() { return redis_server_; }
  const std::vector<GroupInformation>& GetGroupInfo() {
    return group_info_;
  }
  uint64_t GetDiffTimeMax() { return diff_time_max_; }
  uint64_t GetLoopInterval() { return loop_interval_; }

private:
  // Private instance to avoid instancing.
  Configuration();
  ~Configuration() = default;

  // Settings.
  RedisServerInformation redis_server_;
  std::vector<GroupInformation> group_info_;
  uint64_t diff_time_max_;
  uint64_t loop_interval_;
};

#endif  // CONFIGURATION_H_