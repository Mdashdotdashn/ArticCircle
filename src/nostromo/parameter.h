#pragma once

#include "estd.h"

#include <map>
#include <tuple>

//------------------------------------------------------------------------------

namespace detail
{
  template <typename T>
  struct PropertyBase
  {
    using Value = T;

    void setValue(T value) { value_ = std::move(value); }

    T value_ = {};
  };
} // detail

//------------------------------------------------------------------------------

template <class T, class = void>
struct Property;

template <>
struct Property<int, void> : detail::PropertyBase<int>
{
  void setRange(int min, int max)
  {
    min_ = min;
    max_ = max;
  }

  int min_ = 0;
  int max_ = 1;
};

template <typename T>
struct EnumProperty : public detail::PropertyBase<T>
{
  void setEnumStrings(std::map<T, const char *> strings)
  {
    enumStrings_ = std::move(strings);
  }

//  std::string stringValue()
//  {
//    return enumStrings_[value_];
//  }

  std::map<T, const char *> enumStrings_;
};


template <typename T>
typename estd::EnableIfEnum<T>
wrap(const T& value, int direction)
{
  return T((int(value) + direction + int(T::COUNT)) % int(T::COUNT));
};


template <class ... Ts>
struct PropertySet
{
  inline constexpr std::size_t size()
  {
    return std::tuple_size<std::tuple<Ts...>>::value;
  }

  template <class T>
  T& get()
  {
    return std::get<T>(set_);
  }
  std::tuple<Ts...> set_;
};

template<class Model>
struct PropertyManager
{
  PropertyManager()
  {
    cursor_ = 0;
  }

  void next()
  {
    const auto size = properties_.size();
    cursor_ = (cursor_ + 1) % size;
  }

  void updateCurrent(int direction)
  {
    const auto updateFn = [direction](auto &&v)
    {
      v.update(direction);
    };
    estd::visit_at(properties_.set_, cursor_, updateFn);
  }

  typename Model::Properties properties_;
  int cursor_;
};
