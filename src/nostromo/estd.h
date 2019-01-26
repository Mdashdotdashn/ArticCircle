#pragma once

#include <type_traits>
#include <tuple>

namespace detail
{
    template<std::size_t I>
    struct visit_impl
    {
        template<typename Tuple, typename F, typename ...Args>
        inline static constexpr int visit(Tuple const &tuple, std::size_t idx, F fun, Args &&...args) noexcept(noexcept(fun(std::get<I - 1U>(tuple), std::forward<Args>(args)...)) && noexcept(visit_impl<I - 1U>::visit(tuple, idx, fun, std::forward<Args>(args)...)))
        {
            return (idx == (I - 1U) ? (fun(std::get<I - 1U>(tuple), std::forward<Args>(args)...), void(), 0) : visit_impl<I - 1U>::visit(tuple, idx, fun, std::forward<Args>(args)...));
        }

        template<typename R, typename Tuple, typename F, typename ...Args>
        inline static constexpr R visit(Tuple const &tuple, std::size_t idx, F fun, Args &&...args) noexcept(noexcept(fun(std::get<I - 1U>(tuple), std::forward<Args>(args)...)) && noexcept(visit_impl<I - 1U>::template visit<R>(tuple, idx, fun, std::forward<Args>(args)...)))
        {
            return (idx == (I - 1U) ? fun(std::get<I - 1U>(tuple), std::forward<Args>(args)...) : visit_impl<I - 1U>::template visit<R>(tuple, idx, fun, std::forward<Args>(args)...));
        }
    };

    template<>
    struct visit_impl<0U>
    {
        template<typename Tuple, typename F, typename ...Args>
        inline static constexpr int visit(Tuple const&, std::size_t, F, Args&&...) noexcept
        {
            return 0;
        }

        template<typename R, typename Tuple, typename F, typename ...Args>
        inline static constexpr R visit(Tuple const&, std::size_t, F, Args&&...) noexcept(noexcept(R{}))
        {
            static_assert(std::is_default_constructible<R>::value, "Explicit return type of visit_at method must be default-constructible");
            return R{};
        }
    };
}

namespace estd
{
  template<typename Tuple, typename F, typename ...Args>
  inline constexpr void visit_at(Tuple const &tuple, std::size_t idx, F fun, Args &&...args) noexcept(noexcept(detail::visit_impl<std::tuple_size<Tuple>::value>::visit(tuple, idx, fun, std::forward<Args>(args)...)))
  {
      detail::visit_impl<std::tuple_size<Tuple>::value>::visit(tuple, idx, fun, std::forward<Args>(args)...);
  }

  template<typename R, typename Tuple, typename F, typename ...Args>
  inline constexpr R visit_at(Tuple const &tuple, std::size_t idx, F fun, Args &&...args) noexcept(noexcept(detail::visit_impl<std::tuple_size<Tuple>::value>::template visit<R>(tuple, idx, fun, std::forward<Args>(args)...)))
  {
      return detail::visit_impl<std::tuple_size<Tuple>::value>::template visit<R>(tuple, idx, fun, std::forward<Args>(args)...);
  }

  template <bool _Test,
    class _Ty = void>
    using enable_if_t = typename std::enable_if<_Test, _Ty>::type;

  template <typename T>
  using EnableIfEnum = enable_if_t<std::is_enum<T>::value, T>;

} // estd
