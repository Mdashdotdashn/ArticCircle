#include "ac_properties.h"

#include "../../../OC_scales.h"
#include "../../../OC_strings.h"

namespace detail
{
  IntConverterBase::IntConverterBase(Property<int>& p)
  : detail::StringConverterBase<int>(p)
  {
  }
}

String PitchClassStringConverter::propertyToString(const Property<int>& p)
{
  return OC::Strings::note_names[p.value_];
}

String ScaleStringConverter::propertyToString(const Property<int>& p)
{
  return OC::scale_names_short[p.value_];
}

RootNoteProperty::RootNoteProperty()
{
  setValue(0);
  setRange(0, 11);
}

ScaleProperty::ScaleProperty()
{
  setValue(5);
  setRange(0, OC::Scales::NUM_SCALES - 1);
}
