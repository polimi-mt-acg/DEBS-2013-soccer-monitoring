#ifndef SOCCER_MONITORING_DISTANCE_HPP
#define SOCCER_MONITORING_DISTANCE_HPP

#include "details/distance_impl.hpp"

#include <cmath>
#include <tuple>

namespace game {
namespace distance {
/**
 * Compute the euclidean distance of 2 n-dimensional tuples.
 *
 * Formally, let x = (x_1, x_2, ..., x_n) and y = (y_1, y_2, ..., y_n) bet the
 * input tuples. The output is the squared root of the sum from i in 1...n of
 * pow(x_i - y-i, 2).
 *
 * @tparam Ts The tuple element types
 * @param x The first input tuple
 * @param y The second input tuple
 * @return the euclidean distance between x and y
 */
template <typename... Ts>
decltype(auto) euclidean(std::tuple<Ts...> const &x,
                         std::tuple<Ts...> const &y) {
  auto ss = details::sum_of_squares(x, y);
  return std::pow(ss, 0.5f);
}
} // namespace distance
} // namespace game
#endif // SOCCER_MONITORING_DISTANCE_HPP
