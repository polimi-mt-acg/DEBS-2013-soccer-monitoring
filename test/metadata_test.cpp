#include "catch.hpp"
#include "metadata.hpp"

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
}