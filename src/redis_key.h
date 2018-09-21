#ifndef REDIS_KEY_H_
#define REDIS_KEY_H_

// Channels.
extern const char kPEConfigChannel[];
extern const char kFairValueChannel[];

// Keys.
// Prefix of fair value json object. (Key = prefix + symbol)
extern const char kFairValuePrefix[];
// Keys inside fair value json object.
extern const char kTimestampKey[];
extern const char kFairValueKey[];
extern const char kFairValueMVKey[];
extern const char kStdDevRatioKey[];

// Prefix of PE configuration object. (Key = prefix + symbol)
extern const char kPEConfigPrefix[];
// Keys inside PE configuration json object.
extern const char kCalculationMethodKey[];
extern const char kSourcePercentageKey[];
extern const char kSourceTypeKey[];
extern const char kFilterRatioKey[];
extern const char kLotLimitKey[];
extern const char kFixedPrice[];
extern const char kBaseCurrency[];
extern const char kSkewKey[];
extern const char kSkewActiveKey[];
extern const char kSkewTypeKey[];
extern const char kSkewValueKey[];
extern const char kSkewPercentKey[];
extern const char kMovingAverageKey[];

#endif  // REDIS_KEY_H_