#include "catch.hpp"
#include "context.hpp"
#include "event.hpp"
#include "event_fetcher.hpp"
#include "metadata.hpp"
#include "stream_types.hpp"
#include "test_dataset.hpp"

#include <game_statistics.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <visualizer.hpp>

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

    auto events_before_first_pause = 5;
    const auto &first_batch = *fetcher.parse_batch().data;

    // Batch has size equal to number of events before first game interruption
    REQUIRE(first_batch.size() == events_before_first_pause);

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

// TEST_CASE("Fetchers with different time units make the same batches") {
//  std::size_t batch_size = 1500;
//  auto maximum_distance = 5.0;
//
//  //= --------------------- Setup game::Context -----------------------------
//  =
//  /// 1
//  auto meta_1 = game::parse_metadata_string(metadata);
//  auto &players_1 = meta_1.players;
//  auto &teams_1 = meta_1.teams;
//  auto &balls_1 = meta_1.balls;
//
//  auto context_1 = game::Context{};
//  context_1.set_player_map(players_1);
//  context_1.set_team_map(teams_1);
//  context_1.set_ball_map(balls_1);
//
//  for (auto &position : meta_1.positions) {
//    auto sids = std::visit([](auto &&pos) { return pos.get_sids(); },
//    position); context_1.add_position(position, sids);
//  }
//
//  /// 2
//  auto meta_2 = game::parse_metadata_string(metadata);
//  auto &players_2 = meta_2.players;
//  auto &teams_2 = meta_2.teams;
//  auto &balls_2 = meta_2.balls;
//
//  auto context_2 = game::Context{};
//  context_2.set_player_map(players_2);
//  context_2.set_team_map(teams_2);
//  context_2.set_ball_map(balls_2);
//
//  for (auto &position : meta_2.positions) {
//    auto sids = std::visit([](auto &&pos) { return pos.get_sids(); },
//    position); context_2.add_position(position, sids);
//  }
//
//  //= ----------------------- Testing logic -------------------------------- =
//  auto fetcher_1 = game::EventFetcher{
//      GAME_DATA_START_10_1e7, game::file_stream{}, 1, batch_size, context_1};
//  auto stats_1 = game::GameStatistics{maximum_distance, context_1};
//
//  auto fetcher_2 = game::EventFetcher{
//      GAME_DATA_START_10_1e7, game::file_stream{}, 2, batch_size, context_2};
//  auto stats_2 = game::GameStatistics{maximum_distance, context_2};
//
//  auto players = players_1.get_player_names();
//  auto data_1 = std::vector<game::PositionEvent>{};
//  auto data_2 = std::vector<game::PositionEvent>{};
//  auto seconds = 0;
//  auto checkpoints_1 = std::vector<decltype(data_1)>{};
//  auto checkpoints_2 = std::vector<decltype(data_1)>{};
//  while (seconds < 6) {
//    auto batch_1 = fetcher_1.parse_batch();
//    auto batch_2 = fetcher_2.parse_batch();
//
//    std::copy(batch_1.data->begin(), batch_1.data->end(),
//              std::back_inserter(data_1));
//    std::copy(batch_2.data->begin(), batch_2.data->end(),
//              std::back_inserter(data_2));
//
//    if (batch_1.is_period_last_batch) {
//      checkpoints_1.push_back(data_1);
//      data_1.clear();
//    }
//
//    if (batch_2.is_period_last_batch) {
//      checkpoints_2.push_back(data_2);
//      data_2.clear();
//    }
//
//    if (batch_1.is_period_last_batch) {
//      ++seconds;
//    }
//  }
//
//  REQUIRE(checkpoints_1.size() == 2 * checkpoints_2.size());
//  for (std::size_t i = 0; i < checkpoints_1.size(); i += 2) {
//    REQUIRE(checkpoints_1[i].size() + checkpoints_1[i + 1].size() ==
//            checkpoints_2[i / 2].size());
//    auto merge = std::vector<game::PositionEvent>{checkpoints_1[i]};
//    std::copy(checkpoints_1[i + 1].begin(), checkpoints_1[i + 1].end(),
//              std::back_inserter(merge));
//    for (std::size_t j = 0; j < merge.size(); ++j) {
//      REQUIRE(merge[j].get_timestamp() ==
//              checkpoints_2[i / 2][j].get_timestamp());
//    }
//  }
//}