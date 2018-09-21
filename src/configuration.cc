#include "configuration.h"

#include "common/config_file_parser.h"
#include "configuration_key.h"

namespace {

// Configuration file name.
const char kConfigurationFileName[] = "pe.ini";

}

// static
Configuration* Configuration::GetInstance() {
  // Magic statics.
  static Configuration instance;
  return &instance;
}

Configuration::Configuration() {
  // LoadConfig(kConfigurationFileName);
}

void Configuration::LoadConfig(const std::string& file_name) {
  // Use ConfigFileParser to get all settings from configuration file.
  common::ConfigFileParser config_file_parser(file_name);

  // Get redis information.
  redis_server_.host = config_file_parser.GetValue(kRedisServerHost);
  redis_server_.port = config_file_parser.GetInt(kRedisServerPort);
  redis_server_.password = config_file_parser.GetValue(kRedisServerPassword);

  // Get all group settings.
  int group_number = config_file_parser.GetInt(kGroupNumber);
  for (int i = 0; i < group_number; i++) {
    std::string prefix = std::string(kGroupPrefix) + std::to_string(i);
    GroupInformation group_info;
    group_info.base_symbol =
        config_file_parser.GetValue(prefix + kBaseSymbolKey);
    group_info.price_sources =
        config_file_parser.GetListString(prefix + kPriceSourcesKey);
    group_info.symbols =
        config_file_parser.GetListString(prefix + kSymbolsKey);

    group_info_.push_back(group_info);
  }

  // Other settings.
  diff_time_max_ = config_file_parser.GetInt(kDiffTimeMax);
  loop_interval_ = config_file_parser.GetInt(kLoopInterval);
}
