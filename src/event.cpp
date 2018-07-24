#include "event.hpp"

namespace game {
std::ostream &operator<<(std::ostream &os, PositionEvent const &event) {
  os << "sid: " << event.sid << ", timestamp: " << event.timestamp.count()
     << ", x: " << event.x << ", y: " << event.y << ", z: " << event.z;
  return os;
}
} // namespace game