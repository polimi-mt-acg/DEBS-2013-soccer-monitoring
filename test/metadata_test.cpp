#include "catch.hpp"
#include "metadata.hpp"
#include <stdexcept>

TEST_CASE("PlayerMap correctly stores metadata", "[metadata]") {
  using namespace std::literals;

  auto player_1 = "Player 1"s;
  auto player_2 = "Player 2"s;

  auto p1_sid_1 = 1;
  auto p1_sid_2 = 2;
  auto p2_sid = 3;

  auto non_player_sid = 4;

  // Add sensors to players
  auto players = game::PlayerMap();
  players.add_sensor(p1_sid_1, player_1);
  players.add_sensor(p1_sid_2, player_1);
  players.add_sensor(p2_sid, player_2);

  // Check if sensors are stored correctly
  REQUIRE(players.is_player(p1_sid_1));
  REQUIRE(players.is_player(p1_sid_2));
  REQUIRE(players.is_player(p2_sid));

  // Check if non_player_sid is reported as non player sensor
  REQUIRE(!players.is_player(non_player_sid));

  // Player names are stored correctly
  REQUIRE(player_1 == players[p1_sid_1]);
  REQUIRE(player_1 == players[p1_sid_2]);
  REQUIRE(player_2 == players[p2_sid]);
  REQUIRE_THROWS_AS(players[non_player_sid], std::out_of_range);
}

TEST_CASE("BallMap correctly stores metadata", "[metadata]") {
  auto b1_sid = 1;
  auto b2_sid = 2;

  auto non_ball_sid = 3;

  // Add sensors to balls
  auto balls = game::BallMap();
  balls.add_ball(b1_sid);
  balls.add_ball(b2_sid);

  // Check if sensors are stored correctly
  REQUIRE(balls.is_ball(b1_sid));
  REQUIRE(balls.is_ball(b2_sid));

  // Check if non_ball_sid is reportes as non ball sensor
  REQUIRE(!balls.is_ball(non_ball_sid));
}