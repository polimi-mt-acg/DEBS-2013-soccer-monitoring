//
// Created by leonardoarcari on 27/07/18.
//

#ifndef SOCCER_MONITORING_DISTANCE_HPP
#define SOCCER_MONITORING_DISTANCE_HPP

#include <cmath>
#include <tuple>

namespace game {
namespace distance {
namespace details {

template <std::size_t I = 0, typename... Ts>
decltype(auto) sum_of_squares(std::tuple<Ts...> const &x,
                              std::tuple<Ts...> const &y) {
  if constexpr (I == sizeof...(Ts)) {
    using T = typename std::tuple_element_t<(I - 1), std::tuple<Ts...>>;
    return T{};
  } else if constexpr (I < sizeof...(Ts)) {
    auto x_i = std::get<I>(x);
    auto y_i = std::get<I>(y);
    auto sq_diff = std::pow(x_i - y_i, 2.0f);
    return sq_diff + sum_of_squares<I + 1, Ts...>(x, y);
  }
}

} // namespace details

template <typename... Ts>
decltype(auto) euclidean(std::tuple<Ts...> const &x,
                         std::tuple<Ts...> const &y) {
  auto ss = details::sum_of_squares(x, y);
  return std::pow(ss, 0.5f);
}
} // namespace distance
} // namespace game
#endif // SOCCER_MONITORING_DISTANCE_HPP
