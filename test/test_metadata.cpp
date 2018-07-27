#include "catch.hpp"
#include "metadata.hpp"
#include "test_dataset.hpp"

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

TEST_CASE("Parse metadata") {
  auto meta = game::parse_metadata_string(metadata);
  auto &players = meta.players;
  auto &teams = meta.teams;
  auto &balls = meta.balls;

  // Check balls metadata
  REQUIRE(balls.is_ball(4));
  REQUIRE(balls.is_ball(8));
  REQUIRE(balls.is_ball(10));
  REQUIRE(balls.is_ball(12));

  // Player names
  auto gertje = "Nick Gertje"s;
  auto waelzlein = "Niklas Waelzlein"s;
  auto sommer = "Wili Sommer"s;
  auto harlass = "Philipp Harlass"s;
  auto hartleb = "Roman Hartleb"s;
  auto engelhardt = "Erik Engelhardt"s;
  auto schneider = "Sandro Schneider"s;
  auto krapf = "Leon Krapf"s;
  auto baer = "Kevin Baer"s;
  auto ziegler = "Luca Ziegler"s;
  auto mueller = "Ben Mueller"s;
  auto reitstetter = "Vale Reitstetter"s;
  auto lee = "Christopher Lee"s;
  auto heinze = "Leon Heinze"s;
  auto langhans = "Leo Langhans"s;

  // Check players team
  REQUIRE(teams[gertje] == game::Team::A);
  REQUIRE(teams[waelzlein] == game::Team::A);
  REQUIRE(teams[sommer] == game::Team::A);
  REQUIRE(teams[harlass] == game::Team::A);
  REQUIRE(teams[hartleb] == game::Team::A);
  REQUIRE(teams[engelhardt] == game::Team::A);
  REQUIRE(teams[schneider] == game::Team::A);
  REQUIRE(teams[krapf] == game::Team::B);
  REQUIRE(teams[baer] == game::Team::B);
  REQUIRE(teams[ziegler] == game::Team::B);
  REQUIRE(teams[mueller] == game::Team::B);
  REQUIRE(teams[reitstetter] == game::Team::B);
  REQUIRE(teams[lee] == game::Team::B);
  REQUIRE(teams[heinze] == game::Team::B);
  REQUIRE(teams[langhans] == game::Team::B);

  // Check players sensors
  REQUIRE(players[13] == gertje);
  REQUIRE(players[14] == gertje);
  REQUIRE(players[97] == gertje);
  REQUIRE(players[98] == gertje);

  REQUIRE(players[49] == waelzlein);
  REQUIRE(players[88] == waelzlein);

  REQUIRE(players[19] == sommer);
  REQUIRE(players[52] == sommer);

  REQUIRE(players[53] == harlass);
  REQUIRE(players[54] == harlass);

  REQUIRE(players[23] == hartleb);
  REQUIRE(players[24] == hartleb);

  REQUIRE(players[57] == engelhardt);
  REQUIRE(players[58] == engelhardt);

  REQUIRE(players[59] == schneider);
  REQUIRE(players[28] == schneider);

  REQUIRE(players[61] == krapf);
  REQUIRE(players[62] == krapf);
  REQUIRE(players[99] == krapf);
  REQUIRE(players[100] == krapf);

  REQUIRE(players[63] == baer);
  REQUIRE(players[64] == baer);

  REQUIRE(players[65] == ziegler);
  REQUIRE(players[66] == ziegler);

  REQUIRE(players[67] == mueller);
  REQUIRE(players[68] == mueller);

  REQUIRE(players[69] == reitstetter);
  REQUIRE(players[38] == reitstetter);

  REQUIRE(players[71] == lee);
  REQUIRE(players[40] == lee);

  REQUIRE(players[73] == heinze);
  REQUIRE(players[74] == heinze);

  REQUIRE(players[75] == langhans);
  REQUIRE(players[44] == langhans);
}