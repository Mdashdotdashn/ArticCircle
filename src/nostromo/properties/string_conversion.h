#pragma once

#include "Arduino.h"

#include "estd.h"
#include "property.h"

class IntStringConverter
{
public:
  IntStringConverter(int min, int max);
  String toString(int value);
private:
  int min_;
  int max_;
};

class FloatStringConverter
{
public:
  FloatStringConverter(float min, float max);
  String toString(float value);
private:
  float min_;
  float max_;
};

// -----------------------------------------------------------------------------

namespace detail
{
  class IStringConverter
  {
  public:
    virtual String Render() { return "???";};
  };

  template <class T>
  class StringConverterBase: public IStringConverter
  {
  public:
    StringConverterBase(const Property<T>& p)
    : property_(p)
    {}

    String Render() final
    {
      String label = property_.label_;
      if (label.length() > 0) label+= "=";
       return label + propertyToString(property_);
    }

    virtual String propertyToString(const Property<T>& ) = 0;

  private:
    const Property<T>& property_;
  };
}

//------------------------------------------------------------------------------

namespace conversion
{
  // Generic - error
  template <class T, class = void>
  class StringConverter;

  //------------------------------------------------------------------------------

  template <>
  class StringConverter<float, void> : public FloatStringConverter, public detail::StringConverterBase<float>
  {
  public:
    StringConverter(Property<float>& p)
    : FloatStringConverter(p.min_, p.max_)
    , detail::StringConverterBase<float>(p)
    {
    }

    String propertyToString(const Property<float>& p) final
    {
      return FloatStringConverter::toString(p.value_);
    }
  };

  //------------------------------------------------------------------------------

  template <>
  class StringConverter<int, void> : public IntStringConverter, public detail::StringConverterBase<int>
  {
  public:
    StringConverter(Property<int>& p)
    : IntStringConverter(p.min_, p.max_)
    , detail::StringConverterBase<int>(p)
    {
    }

    String propertyToString(const Property<int>& p) final
    {
      return IntStringConverter::toString(p.value_);
    }

  };

  //------------------------------------------------------------------------------

  template <class T>
  class StringConverter<T, estd::EnableIfEnum<T>> : public detail::StringConverterBase<T>
  {
  public:
    StringConverter(Property<T>& p)
    : detail::StringConverterBase<T>(p)
    , enumStrings_(std::move(p.enumStrings_))
    {}

    String propertyToString(const Property<T>& p) final
    {
      return enumStrings_[std::size_t(p.value_)];
    }

    typename Property<T>::NameStore enumStrings_;
  };

} // namespace conversion

//------------------------------------------------------------------------------

// Generic case, build a string converted based on the proerty's value type
template <class Property, class Enable = void>
struct StringConverter
{
  using type = conversion::StringConverter<typename Property::value_t>;
};

//------------------------------------------------------------------------------

template <class Property>
struct StringConverter<Property, estd::EnableIfTypeExists<typename Property::StringConverter>>
{
  using type = typename Property::StringConverter;
};


//------------------------------------------------------------------------------

template <class Property>
using string_converter_t = typename StringConverter<Property>::type;


template <class Property>
auto makeStringConverter(const Property& p)
{
  using Converter = string_converter_t<Property>;
  return Converter{p};
};
