#include "catch.hpp"

#include "distance.hpp"
#include <cmath>

TEST_CASE("Test sum of squares") {
  auto null_vector = std::make_tuple(0.0, 0.0, 0.0);
  auto one_vector = std::make_tuple(1.0, 1.0, 1.0);
  auto neg_vector = std::make_tuple(-1.0, -1.0, -1.0);

  REQUIRE(game::distance::details::sum_of_squares(null_vector, one_vector) ==
          3.0);
  REQUIRE(game::distance::details::sum_of_squares(null_vector, null_vector) ==
          0.0);
  REQUIRE(game::distance::details::sum_of_squares(one_vector, one_vector) ==
          0.0);
  REQUIRE(game::distance::details::sum_of_squares(one_vector, neg_vector) ==
          12.0);
}

TEST_CASE("Test Euclidean Distance") {
  auto null_vector = std::make_tuple(0.0, 0.0, 0.0);
  auto one_vector = std::make_tuple(1.0, 1.0, 1.0);
  auto neg_vector = std::make_tuple(-1.0, -1.0, -1.0);

  REQUIRE(game::distance::euclidean(null_vector, one_vector) ==
      std::pow(3.0, 0.5));
  REQUIRE(game::distance::euclidean(null_vector, null_vector) ==
      std::pow(0.0, 0.5));
  REQUIRE(game::distance::euclidean(one_vector, one_vector) ==
      std::pow(0.0, 0.5));
  REQUIRE(game::distance::euclidean(one_vector, neg_vector) ==
      std::pow(12.0, 0.5));
}