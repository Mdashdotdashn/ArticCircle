#pragma once

#include "../properties/property_bundle.h"

namespace detail
{
  template <class Interface,template <class...> class Concrete, class... Args>
  auto mkarray()
  {
    std::array<std::unique_ptr<Interface>, sizeof...(Args)> a = { std::make_unique<Concrete<Args>>()... };
    return a;
  };

  // Utilities to convert a property class to an index within the array

  template <class T, class Tuple>
  struct Index;

  template <class T, class... Types>
  struct Index<T, std::tuple<T, Types...>> {
      static const std::size_t value = 0;
  };

  template <class T, class U, class... Types>
  struct Index<T, std::tuple<U, Types...>> {
      static const std::size_t value = 1 + Index<T, std::tuple<Types...>>::value;
  };
} // detail


template <typename... Props>
class PropertySet
{
public:
  // Construct

  PropertySet()
  {
    bundles_ = detail::mkarray<IPropertyBundle, PropertyBundle, Props...>();
  }

  // Index based accessor
  IPropertyBundle& getBundle(int index)
  {
    return *bundles_[index];
  }

  // Property based accessor
  template <class Property>
  IPropertyBundle& getBundle()
  {
    constexpr auto index = detail::Index<Property, std::tuple<Props...>>::value;
    return getBundle(index);
  }

  constexpr static std::size_t size()
  {
    return sizeof...(Props);
  }

private:
  std::array<std::unique_ptr<IPropertyBundle> ,sizeof...(Props)> bundles_;
};
