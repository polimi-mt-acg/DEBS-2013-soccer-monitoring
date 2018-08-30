#include <filesystem>
#include <iostream>
#include <omp.h>

#include <boost/program_options.hpp>
#include <fstream>

#include "soccer_monitoring.hpp"

#include "fmt/format.h"

struct Arguments {
  int time_units;
  double maximum_distance;
  std::filesystem::path game_data;
  std::filesystem::path metadata;
  int nb_threads;
  std::size_t batch_size;
  std::filesystem::path output;
};

Arguments parse_arguments(int argc, char *argv[]) {
  namespace po = boost::program_options;
  namespace fs = std::filesystem;

  po::options_description desc("DEBS 2013 - Soccer Monitoring tool");
  desc.add_options()("help,h", "Print this message")(
      "time-units,T", po::value<int>(), "Frequency of statistics (in seconds)")(
      "max-distance,K", po::value<double>(),
      "Maximum distance for ball possession eligibility")(
      "stream,s", po::value<std::string>(), "Game stream file path")(
      "metadata,m", po::value<std::string>(), "Metadata file path")(
      "threads,t", po::value<int>()->default_value(0), "Number of threads")(
      "batch-size,B", po::value<int>()->default_value(1500),
      "Events batch size (default: auto)")(
      "output,o", po::value<std::string>(),
      "Output file path (default: stdout)");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    std::exit(1);
  }

  int time_units = 0;
  if (vm.count("time-units")) {
    time_units = vm["time-units"].as<int>();
    if (time_units < 1 || time_units > 60) {
      fmt::print("Invalid value for --time-units: {}. Valid range: [1, 60]",
                 time_units);
      std::exit(1);
    }
  } else {
    std::cout << "Missing mandatory argument: --time-units\n" << desc;
    std::exit(1);
  }

  double max_distance = 0.0;
  if (vm.count("max-distance")) {
    max_distance = vm["max-distance"].as<double>();
    if (max_distance < 1.0 || max_distance > 5.0) {
      fmt::print(
          "Invalid value for --max-distance: {}. Valid range: [1.0, 5.0]",
          max_distance);
      std::exit(1);
    }
  } else {
    std::cout << "Missing mandatory argument: --max-distance\n" << desc;
    std::exit(1);
  }

  auto game_data = fs::path{};
  if (vm.count("stream")) {
    game_data = fs::path{vm["stream"].as<std::string>()};
    if (!fs::is_regular_file(game_data) || fs::is_empty(game_data)) {
      fmt::print("Invalid value for --stream. {} file not found.",
                 game_data.string());
      std::exit(1);
    }
  } else {
    std::cout << "Missing mandatory argument: --stream\n" << desc;
    std::exit(1);
  }

  auto metadata = fs::path{};
  if (vm.count("metadata")) {
    metadata = fs::path{vm["metadata"].as<std::string>()};
    if (!fs::is_regular_file(metadata) || fs::is_empty(metadata)) {
      fmt::print("Invalid value for --metadata. {} file not found.",
                 metadata.string());
      std::exit(1);
    }
  } else {
    std::cout << "Missing mandatory argument: --metadata\n" << desc;
    std::exit(1);
  }

  int nb_threads = 0;
  if (vm.count("threads")) {
    nb_threads = vm["threads"].as<int>();
    if (nb_threads < 0) {
      fmt::print(
          "Invalid value for --threads: {}. Must be greater or equal to 0",
          nb_threads);
      std::exit(1);
    }

    if (nb_threads == 0) {
      nb_threads = omp_get_max_threads();
    }

    if (nb_threads > omp_get_max_threads()) {
      fmt::print("WARNING: You are setting a number of threads ({}) greater "
                 "than the number of logical threads of your machine ({}). "
                 "Performance will degrade.",
                 nb_threads, omp_get_max_threads());
    }
  } else {
    std::cout << "Missing mandatory argument: --threads\n" << desc;
    std::exit(1);
  }

  int tmp_batch_size = 0;
  std::size_t batch_size = 0;
  if (vm.count("batch-size")) {
    tmp_batch_size = vm["batch-size"].as<int>();
    if (tmp_batch_size < 1) {
      fmt::print("Invalid value for --batch-size: {}. Must be greater than 0",
                 tmp_batch_size);
      std::exit(1);
    } else {
      batch_size = static_cast<std::size_t>(tmp_batch_size);
    }
  }

  auto output = fs::path{};
  if (vm.count("output")) {
    output = vm["output"].as<std::string>();
  }

  return {time_units, max_distance, game_data, metadata,
          nb_threads, batch_size,   output};
}

int main(int argc, char *argv[]) {
  auto args = parse_arguments(argc, argv);

  if (args.output.empty()) {
    // Output to stdout
    game::run_game_monitoring(args.time_units, args.maximum_distance,
                              args.game_data, args.metadata, args.nb_threads,
                              args.batch_size);
  } else {
    // Output to file
    game::run_game_monitoring(args.time_units, args.maximum_distance,
                              args.game_data, args.metadata, args.nb_threads,
                              args.batch_size, args.output.string());
  }
}
