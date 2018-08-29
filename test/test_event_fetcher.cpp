#include "catch.hpp"
#include "context.hpp"
#include "event.hpp"
#include "event_fetcher.hpp"
#include "metadata.hpp"
#include "stream_types.hpp"
#include "test_dataset.hpp"

#include <iostream>
#include <sstream>
#include <string>

TEST_CASE("Test Event Fetcher", "[event_fetcher]") {
  using namespace std::literals;

  SECTION("Test dataset event parsing with Regex") {
    const auto event_line =
        "SE,69,10632029737813340,27679,-221,1011,553570,2481132,-9441,2048,2580,-8913,1107,4396"s;
    auto expected_timestamp = std::chrono::picoseconds{10632029737813340};
    auto event = game::parse_event_line(event_line, game::parser_regex{});
    REQUIRE(std::holds_alternative<game::PositionEvent>(event));

    auto position_event = std::get<game::PositionEvent>(event);
    REQUIRE(69 == position_event.get_sid());
    REQUIRE(expected_timestamp == position_event.get_timestamp());
    REQUIRE(27679 == position_event.get_x());
    REQUIRE(-221 == position_event.get_y());
    REQUIRE(1011 == position_event.get_z());
  }

  SECTION("Test dataset event parsing with Custom") {
    const auto event_line =
        "SE,69,10632029737813340,27679,-221,1011,553570,2481132,-9441,2048,2580,-8913,1107,4396"s;
    auto expected_timestamp = std::chrono::picoseconds{10632029737813340};
    auto event = game::parse_event_line(event_line, game::parser_custom{});
    REQUIRE(std::holds_alternative<game::PositionEvent>(event));

    auto position_event = std::get<game::PositionEvent>(event);
    REQUIRE(69 == position_event.get_sid());
    REQUIRE(expected_timestamp == position_event.get_timestamp());
    REQUIRE(27679 == position_event.get_x());
    REQUIRE(-221 == position_event.get_y());
    REQUIRE(1011 == position_event.get_z());
  }

  SECTION("Test events parsing") {
    const auto position_events =
        "SE,69,10632029737813340,27679,-221,1011,553570,2481132,-9441,2048,"
        "2580,-"
        "8913,1107,4396\n"
        "SE,19,10633811911716270,26416,-6156,258,141603,523633,7167,1041,6895,"
        "3463,-7555,5560\n"
        "SE,67,10634726820637202,26679,-578,197,170867,685864,1416,-1141,-9833,"
        "8312,-1441,-5369\n"
        "SE,67,10634731737624712,26679,-580,194,140437,607509,2074,-855,-9744,"
        "9661,2063,-1549\n"
        "SE,66,10634732514566388,27813,-558,67,142069,1233916,-5400,-636,8392,-"
        "158,-7878,6156\n"
        "SE,75,10634735140223609,26254,-1097,-154,82907,837089,-539,9671,2485,"
        "7827,6169,-816\n"
        "SE,67,10634736654612317,26679,-580,192,150530,668588,2205,-473,-9742,"
        "9189,3206,-2296\n"
        "SE,66,10634737426529044,27813,-558,69,121712,1513838,-2211,-52,9752,"
        "7855,-3696,4961\n"
        "SE,65,10634738740592737,28258,-893,439,53656,885053,5930,450,8038,8042,5909,-626\n"s;

    auto meta = game::parse_metadata_string(metadata);
    auto &players = meta.players;
    auto &teams = meta.teams;
    auto &balls = meta.balls;

    auto context = game::Context{};
    context.set_player_map(players);
    context.set_team_map(teams);
    context.set_ball_map(balls);

    for (auto &position : meta.positions) {
      auto sids =
          std::visit([](auto &&pos) { return pos.get_sids(); }, position);
      context.add_position(position, sids);
    }

    int time_units = 90 * 60;
    auto fetcher = game::EventFetcher(position_events, game::string_stream{},
                                      time_units, 10, context);

    // Read all the position events and print them
    for (int i = 0; i < 9; ++i) {
      auto position_event = fetcher.parse_next_event();
      std::cout << position_event << "\n";
    }

    // Another read throws a ios failure (no new line available)
    REQUIRE_THROWS_AS(fetcher.parse_next_event(), std::ios_base::failure);
  }

  SECTION("Test batch parsing") {
    std::size_t batch_size = 10;
    int time_units = 90 * 60;

    auto meta = game::parse_metadata_string(metadata);
    auto &players = meta.players;
    auto &teams = meta.teams;
    auto &balls = meta.balls;

    auto context = game::Context{};
    context.set_player_map(players);
    context.set_team_map(teams);
    context.set_ball_map(balls);

    for (auto &position : meta.positions) {
      auto sids =
          std::visit([](auto &&pos) { return pos.get_sids(); }, position);
      context.add_position(position, sids);
    }

    auto fetcher =
        game::EventFetcher{game_data_start_10_50, game::string_stream{},
                           time_units, batch_size, context};

    const auto &first_batch = fetcher.parse_batch().data.get();

    // Batch has BATCH_SIZE size
    REQUIRE(first_batch.size() == batch_size);

    // Game events before game start are dropped
    REQUIRE(first_batch[0].get_timestamp() >= game::game_start);

    for (auto &event : first_batch) {
      if (balls.is_ball(event.get_sid())) {
        fmt::print("Ball [{}] moved to position ({}, {}, {}) at {}\n",
                   event.get_sid(), event.get_x(), event.get_y(), event.get_z(),
                   event.get_timestamp().count());
      } else if (players.is_player(event.get_sid())) {
        auto name = players[event.get_sid()];
        // auto team = (teams[name] == game::Team::A ? "A"s : "B"s);
        fmt::print("{} [{}] moved to position ({}, {}, {}) at {}\n", name,
                   event.get_sid(), event.get_x(), event.get_y(), event.get_z(),
                   event.get_timestamp().count());
      }
    }
  }
}