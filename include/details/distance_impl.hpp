#ifndef SOCCER_MONITORING_DISTANCE_IMPL_HPP
#define SOCCER_MONITORING_DISTANCE_IMPL_HPP

#include <cmath>
#include <tuple>

namespace game {
namespace distance {
namespace details {
/**
 * Compute the sum of the element-wise squared difference of the input tuples.
 *
 * Formally, let x = (x_1, x_2, ..., x_n) and y = (y_1, y_2, ..., y_n) bet the
 * input tuples. The output is the sum for i in 1...n of pow(x_i - y_i, 2).
 *
 * @tparam I The tuple index
 * @tparam Ts The tuple element types
 * @param x The first input tuple
 * @param y The second input tuple
 * @return sum for i from @p I to n of pow(x[i]-y[i], 2)
 */
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
} // namespace distance
} // namespace game

#endif // SOCCER_MONITORING_DISTANCE_IMPL_HPP
