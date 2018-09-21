#include "redis_key.h"

// Channels.
const char kPEConfigChannel[] = "config_pe_message";
const char kFairValueChannel[] = "price_engine_message";

// Keys.
const char kFairValuePrefix[] = "price_engine_data_";
const char kTimestampKey[] = "timestamp";
const char kFairValueKey[] = "fair_value";
const char kFairValueMVKey[] = "mov_avr";
const char kStdDevRatioKey[] = "std_avr_ratio";

const char kPEConfigPrefix[] = "config_pe_";
const char kCalculationMethodKey[] = "FVType";
const char kSourcePercentageKey[] = "PEpercentage";
const char kSourceTypeKey[] = "";
const char kFilterRatioKey[] = "filter_ratio";
const char kLotLimitKey[] = "PElotlimit";
const char kFixedPrice[] = "FVFixedPrice";
const char kBaseCurrency[] = "base_currency";   // Currently, not used.
const char kSkewKey[] = "PEinput";
const char kSkewActiveKey[] = "active";
const char kSkewTypeKey[] = "type_active";
const char kSkewValueKey[] = "value";
const char kSkewPercentKey[] = "percent";
const char kMovingAverageKey[] = "PEmvlen";
