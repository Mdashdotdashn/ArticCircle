#include "string_conversion.h"

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
