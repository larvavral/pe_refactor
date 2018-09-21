#include <fstream>
#include <thread>

#include "configuration.h"
#include "glog/logging.h"
#include "group.h"
#include "hiredis/adapters/libevent.h"
#include "redis_controller.h"

int main(int argc, const char *argv[]) {
  signal(SIGPIPE, SIG_IGN);

  // Initialize Google's logging library.
  google::SetLogDestination(google::INFO, "./log/PE.log.");
  FLAGS_alsologtostderr = 1;
  google::InitGoogleLogging("PE");

  // Get configuration file name from input parameter.
  if (argc != 2) {
    LOG(ERROR) << "Parameter number is not correct!";
    return EXIT_FAILURE;
  }

  std::string config_file_name(argv[1]);
  // Check whether file is existed or not.
  std::ifstream file(config_file_name);
  if (!file) {
    LOG(ERROR) << "Configuration file is not existed: " << config_file_name;
    return EXIT_FAILURE;
  }

  // Read configuration from file.
  LOG(INFO) << "Read all settings from configuration file.";
  Configuration* configuration = Configuration::GetInstance();
  configuration->LoadConfig(config_file_name);
  RedisServerInformation redis_server = configuration->GetRedisServerInfo();

  // Log all settings here for checking if necessary.
  LOG(INFO) << "Redis server information: \n"
            << " - Host: " << redis_server.host << "\n"
            << " - Port: " << redis_server.port << "\n"
            // << " - Password: " << redis_server.password << "\n"
            ;

  // Initialize.
  // Setup event_base to listening event for async connection.
  // TODO(hoangpq): Currently, we just use one event_base for all async
  // connections. Please consider to use one event_base for each async
  // conntections if there are too many async connections in the future.
  struct event_base* base = event_base_new();

  // Initialize for each group.
  std::vector<std::unique_ptr<Group>> groups;
  int count = 0;
  for (auto& group_info : configuration->GetGroupInfo()) {
    LOG(INFO) << "Group " << ++count << ":";
    std::unique_ptr<Group> group(new Group(redis_server, group_info, base));
    groups.push_back(std::move(group));
  }

  // Run event_base_dispatch() on a seperate thread to do not
  // disturb main thread.
  std::thread async_event_thread = std::thread([&base]() {
    event_base_dispatch(base);
  });

  // Main process.
  // Start generated price loop.
  for (auto& group : groups)
    group->StartLoop();

  // Keep program running until receive exit signal.
  // TODO(hoangpq): Plz catch SIGTERM signal to stop program.
  while (true);

  // Releasing.

  return 0;
}
