#include "details/visualizer_impl.hpp"

namespace game {
namespace details {
bool PartialsCmp::operator()(std::string const &p1,
                             std::string const &p2) const {
  if (p1 == p2)
    return false; // For all a, comp(a, a) == false

  auto team_1 = (*teams)[p1];
  auto team_2 = (*teams)[p2];
  if (team_1 == team_2)
    return p1 < p2;
  else
    return team_1 == Team::A;
}
} // namespace details
} // namespace game