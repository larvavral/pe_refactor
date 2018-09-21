#include "configuration_key.h"

// Redis server information.
const char kRedisServerHost[] = "redis_server.host";
const char kRedisServerPort[] = "redis_server.port";
const char kRedisServerPassword[] = "redis_server.password";

// Group settings.
const char kGroupNumber[] = "group.group_number";
const char kGroupPrefix[] = "group_";
const char kBaseSymbolKey[] = ".base_symbol";
const char kPriceSourcesKey[] = ".price_sources";
const char kSymbolsKey[] = ".symbols";

// Common settings.
const char kDiffTimeMax[] = "common.diff_time_max";
const char kLoopInterval[] = "common.loop_interval";
