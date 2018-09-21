#ifndef CONFIGURATION_KEY_H_
#define CONFIGURATION_KEY_H_

// Contains all keys, which are used to read settings from configuration file.

// Redis server information.
extern const char kRedisServerHost[];
extern const char kRedisServerPort[];
extern const char kRedisServerPassword[];

// Group settings.
extern const char kGroupNumber[];
extern const char kGroupPrefix[];
extern const char kBaseSymbolKey[];
extern const char kPriceSourcesKey[];
extern const char kSymbolsKey[];

// Common settings.
// extern const char kServerType[];
extern const char kDiffTimeMax[];
// Interval time of loop, which generated price for all symbols. (milliseconds)
extern const char kLoopInterval[];

#endif  // CONFIGURATION_KEY_H_