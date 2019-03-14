#pragma once

#include <cmath>

namespace detail
{
  struct IValueConverter
  {
    virtual void update(int direction) = 0;
  };

  template <class T>
  class ValueConverterBase: public IValueConverter
  {
  public:
    ValueConverterBase(Property<T>& p)
    : property_(p)
    {}
  protected:
    Property<T>& property_;
  };
} // detail

//------------------------------------------------------------------------------

namespace conversion
{
  // Generic - error
  template <class T, class = void>
  class ValueConverter;

  // default converters for all types
  template <>
  class ValueConverter<float, void> : public detail::ValueConverterBase<float>
  {
  public:
    ValueConverter(Property<float>& p)
    : detail::ValueConverterBase<float>(p)
    {}

    void update(int direction) override
    {
      property_.setValue(clamp(property_.value_ + float(direction) * property_.increment_, property_.min_, property_.max_));
    }
  };

  template <>
  class ValueConverter<int, void> : public detail::ValueConverterBase<int>
  {
  public:
    ValueConverter(Property<int>& p)
    : detail::ValueConverterBase<int>(p)
    {}

    void update(int direction) override
    {
      property_.setValue(clamp(property_.value_ + direction, property_.min_, property_.max_));
    }
  };

  template <class T>
  class ValueConverter<T, estd::EnableIfEnum<T>> : public detail::ValueConverterBase<T>
  {
  public:
    ValueConverter(Property<T>& p)
    : detail::ValueConverterBase<T>(p)
    {}

    void update(int direction) override
    {
      constexpr auto size = Property<T>::size;
      auto &p = detail::ValueConverterBase<T>::property_;
      const auto value = T((int(p.value_) + direction + size) % size);
      p.setValue(value);
    }
  };

} // conversion


class ExponentialValueConverter
  : public detail::ValueConverterBase<float>
{
public:
  ExponentialValueConverter(Property<float>& p)
  : detail::ValueConverterBase<float>(p)
  {}

  void update(int direction) override
  {
    const auto range = property_.max_ - property_.min_;
    const auto min = property_.min_;
    const auto increment = property_.increment_;
    const auto scaling = property_.scaling_;

    auto value = property_.value_;
    auto internal = std::pow((value - min) / range, 1.0f / scaling);
    internal = clamp(internal + float(direction) * increment, 0.f, 1.f);
    value = std::pow(internal, scaling) * range + min;
    property_.setValue(value);
  }
};

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
