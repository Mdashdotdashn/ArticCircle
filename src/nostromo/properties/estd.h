#pragma once

#include <type_traits>

namespace estd
{
  namespace detail
  {
    template <typename T>
    struct ToVoid
    {
      using type = void;
    };
  } // detail

  template <typename T>
  using EnableIfEnum = std::enable_if_t<std::is_enum<T>::value>;

  template <typename T>
  using EnableIfTypeExists = typename detail::ToVoid<T>::type;
} // estd
