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
  };

  template <typename T>
  struct PropertyBase : IProperty
  {
    using value_t = T;
    using Callback = std::function<void(const T&)>;

    void setValue(T value) { value_ = std::move(value); }
    void setLabel(const String& label) {
      label_ = label;
    };
    void setCallback(const Callback& callback)
    {
      callback_ = callback;
      triggerCallback();
    }

    void triggerCallback()
    {
      if (callback_) callback_(value_);
    }

    Callback callback_;
    String label_;
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
    value_ = clamp(value_ , min_, max_);
    triggerCallback();
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

  void setRange(float min, float max,float increment = 0.f)
  {
    min_ = min;
    max_ = max;
    value_ = clamp(value_ , min_, max_);
    increment_ = (increment == 0.f) ? (max_ - min_) / 50.f : increment_;
    triggerCallback();
  }

  float min_ = 0.f;
  float max_ = 1.f;
  float increment_= 1.f / 50.f;
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

  NameStore enumStrings_;
};

//------------------------------------------------------------------------------
