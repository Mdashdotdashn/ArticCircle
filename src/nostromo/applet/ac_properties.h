#pragma once

#include "../properties/string_conversion.h"

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

//------------------------------------------------------------------------------

struct PercentageProperty : public Property<float>
{
  PercentageProperty()
  {
    setRange(0.f, 1.f);
  }
};

//------------------------------------------------------------------------------

struct OctaveProperty: Property<int>
{
  OctaveProperty()
  {
    setLabel("o");
    setRange(-12,3);
    setValue(-1);
  }
};
