#pragma once

#include "string_conversion.h"

namespace detail
{
  struct IntConverterBase: public detail::StringConverterBase<int>
  {
    IntConverterBase(Property<int>& p);
  };
}

struct PitchClassStringConverter: public detail::IntConverterBase
{
  using IntConverterBase::IntConverterBase;

  virtual String propertyToString(const Property<int>& p) final;
};

struct ScaleStringConverter: public detail::IntConverterBase
{
  using IntConverterBase::IntConverterBase;

  virtual String propertyToString(const Property<int>& p) final;
};


//------------------------------------------------------------------------------

struct RootNoteProperty: Property<int>
{
  RootNoteProperty();
  using StringConverter = PitchClassStringConverter;
};

struct ScaleProperty: Property<int>
{
  ScaleProperty();
  using StringConverter = ScaleStringConverter;
};