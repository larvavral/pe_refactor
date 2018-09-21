#include "common/config_file_parser.h"

#include "common/string_helper.h"
#include <fstream>

namespace common {

ConfigFileParser::ConfigFileParser() {}

ConfigFileParser::ConfigFileParser(const std::string& file_name) {
  ReadFromFile(file_name);
}

ConfigFileParser::~ConfigFileParser() {}

void ConfigFileParser::ReadFromFile(const std::string& file_name) {
  // Remove old data before reading new file.
  data_.clear();

  // Open file.
  std::ifstream file(file_name);
  if (!file) {
    return;
  }

  // Read all settings from file.
  int count = 0;
  std::string line;
  while (std::getline(file, line)) {
    count++;

    // Remove comment. Comment is characters after '#'.
    line = line.substr(0, line.find_first_of("#"));

    // Ignore empty line.
    if (line.empty()) continue;

    std::vector<std::string> vs = common::Split(line, '=');
    if (vs.size() == 2) {
      std::string key = common::Trim(vs[0]);
      std::string value = common::Trim(vs[1]);

      // TODO(hoangpq): Should check whether key is existed or not.
      data_[key] = value;
    }
  }

  // Close file.
  if (file)
    file.close();
}

std::string ConfigFileParser::GetValue(const std::string& key) {
  return data_.find(key) == data_.end() ? std::string() : data_[key];
}

int ConfigFileParser::GetInt(const std::string& key) {
  std::string value = GetValue(key);
  if (value.empty()) return 0;

  size_t pos;
  int result = stoi(value, &pos);
  return pos != value.size() ? 0 : result;
}

double ConfigFileParser::GetDouble(const std::string& key) {
  return 0.0;
}

std::vector<std::string> ConfigFileParser::GetListString(
    const std::string& key) {
  std::string value = GetValue(key);
  return common::Split(value, ',');
}

} // namespace common
