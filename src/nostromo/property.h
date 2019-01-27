#pragma once

#include "estd.h"
#include "math.h"

#include <utility>
#include <memory>

namespace detail
{
  struct IProperty
  {
      virtual void update(int direction) {};
      virtual String stringValue() {return "IProp";};
  };

  template <typename T>
  struct PropertyBase : IProperty
  {
    using value_t = T;
    using Callback = std::function<void(const T&)>;

    void setValue(T value) { value_ = std::move(value); }
    void setCallback(const Callback& callback)
    {
      callback_ = callback;
    }

    void triggerCallback()
    {
      if (callback_) callback_(value_);
    }

    Callback callback_;
    T value_ = {};
  };
} // detail

//------------------------------------------------------------------------------

template <class T, class = void>
struct Property;

template <>
struct Property<int, void> : detail::PropertyBase<int>
{
  void update(int direction) override
  {
    value_ = clamp(value_ + direction, min_, max_);
    triggerCallback();
  }

  void setRange(int min, int max)
  {
    min_ = min;
    max_ = max;
  }

  String stringValue() override
  {
    static char str[16];
    snprintf(str, sizeof(str), "%4d", value_);
    return str;
  }

  int min_ = 0;
  int max_ = 1;
};

//------------------------------------------------------------------------------

template <>
struct Property<float, void>: detail::PropertyBase<float>
{
  void update(int direction) override
  {
    value_ = clamp(value_ + float(direction)/10.f, min_, max_);
    triggerCallback();
  }

  void setRange(int min, int max)
  {
    min_ = min;
    max_ = max;
  }

  String stringValue() override
  {
    static char str[16];
    snprintf(str, sizeof(str), "%10.2f", value_);
    return str;
  }

  float min_ = 0.f;
  float max_ = 1.f;
};

//------------------------------------------------------------------------------

template <typename T>
struct Property<T, estd::EnableIfEnum<T>> : detail::PropertyBase<T>
{
  static constexpr auto size = std::size_t(T::COUNT);

  Property()
  {
    enumStrings_.fill("--");
  }

  using NameStore = std::array<const char *, size>;
  void setEnumStrings(const NameStore& strings)
  {
    enumStrings_ = strings;
  }

  void update(int direction) override
  {
    detail::PropertyBase<T>::value_ = T((int(detail::PropertyBase<T>::value_) + direction + size) % size);
    detail::PropertyBase<T>::triggerCallback();
  }

  String stringValue() override
  {
    return enumStrings_[std::size_t(detail::PropertyBase<T>::value_)];
  }
  NameStore enumStrings_;
};

//------------------------------------------------------------------------------
