#ifndef SOCCER_MONITORING_PARTIALS_CMP_HPP
#define SOCCER_MONITORING_PARTIALS_CMP_HPP

#include <string>

#include "metadata.hpp"

namespace game {
namespace details {
struct PartialsCmp {
  explicit PartialsCmp(TeamMap const &teams) : teams{std::addressof(teams)} {}

  bool operator()(std::string const &p1, std::string const &p2) const;

private:
  TeamMap const *teams;
};
} // namespace details
} // namespace game

#endif // SOCCER_MONITORING_PARTIALS_CMP_HPP
