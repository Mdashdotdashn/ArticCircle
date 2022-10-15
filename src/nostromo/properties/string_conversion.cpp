#include "string_conversion.h"

#include <inttypes.h>

UInt32StringConverter::UInt32StringConverter(uint32_t min, uint32_t max)
: min_(min)
, max_(max)
{}

String UInt32StringConverter::toString(uint32_t value)
{
  static char str[16];
  snprintf(str, sizeof(str), "%08x", value);
  return str;
}

IntStringConverter::IntStringConverter(int min, int max)
: min_(min)
, max_(max)
{}

String IntStringConverter::toString(int value)
{
  static char str[16];
  snprintf(str, sizeof(str), "%4d", value);
  return str;
}

FloatStringConverter::FloatStringConverter(float min, float max)
: min_(min)
, max_(max)
{}

String FloatStringConverter::toString(float value)
{
  static char str[16];
  snprintf(str, sizeof(str), "%4.2f", value);
  return str;
}
