#pragma once

namespace detail
{
  class IValueConverter
  {

  };

  template <class T>
  class ValueConverterBase: public IValueConverter
  {
  public:
    ValueConverterBase(const Property<T>& p)
    : property_(p)
    {}
  private:
    const Property<T>& property_;
  };
} // detail

//------------------------------------------------------------------------------

namespace conversion
{
  // Generic - error
  template <class T, class = void>
  class ValueConverter;

  template <>
  class ValueConverter<float, void> : public detail::ValueConverterBase<float>
  {
  public:
    ValueConverter(Property<float>& p)
    : detail::ValueConverterBase<float>(p)
    {}
  };

  template <>
  class ValueConverter<int, void> : public detail::ValueConverterBase<int>
  {
  public:
    ValueConverter(Property<int>& p)
    : detail::ValueConverterBase<int>(p)
    {}
  };

  template <class T>
  class ValueConverter<T, estd::EnableIfEnum<T>> : public detail::ValueConverterBase<T>
  {
  public:
    ValueConverter(Property<T>& p)
    : detail::ValueConverterBase<T>(p)
    {}
  };

} // conversion

// class LinearConverter
// {
// public:
//     LinearConverter(const float minimum, const float maximum)
//     : minimum_(minimum)
//     , range_(maximum-minimum)
//     {
//     }
//
//     float toInternal(float value)
//     {
//       return (value - minimum) / range_;
//     }
//
//     float fromInternal(float value)
//     {
//       return (value * range_) / minimum_;
//     }
//   private:
//     float minimum_;
//     float range_;
// };

// Generic case, build a string converted based on the proerty's value type
template <class Property, class Enable = void>
struct ValueConverter
{
  using type = conversion::ValueConverter<typename Property::value_t>;
};

//------------------------------------------------------------------------------

template <class Property>
struct ValueConverter<Property, estd::EnableIfTypeExists<typename Property::ValueConverter>>
{
  using type = typename Property::ValueConverter;
};


//------------------------------------------------------------------------------

template <class Property>
using value_converter_t = typename ValueConverter<Property>::type;
