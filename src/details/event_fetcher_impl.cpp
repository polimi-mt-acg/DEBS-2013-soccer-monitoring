#include "details/event_fetcher_impl.hpp"
#include "event_fetcher.hpp"

namespace game {
namespace details {
event_fetcher_iterator::event_fetcher_iterator(EventFetcher &f)
    : fetcher{std::addressof(f)}, is_end{f.game_over},
      value{fetcher->parse_batch()} {}

event_fetcher_iterator::iterator &event_fetcher_iterator::operator++() {
  if (fetcher->game_over) {
    is_end = true;
  } else {
    value = fetcher->parse_batch();
    is_end = false;
  }
  return *this;
}

event_fetcher_iterator::iterator event_fetcher_iterator::operator++(int) {
  auto it = *this;
  this->operator++();
  return it;
}

event_fetcher_iterator::iterator &event_fetcher_iterator::
operator=(const event_fetcher_iterator::iterator &other) {
  fetcher = other.fetcher;
  is_end = other.is_end;
  value = other.value;
  return *this;
}

event_fetcher_iterator::event_fetcher_iterator()
    : fetcher{nullptr}, is_end{false}, value{} {}

void update_sensor_position(Positions &position,
                            PositionEvent const &position_event) {
  std::visit(
      [&position_event](auto &&pos) {
        pos.update_sensor(position_event.get_sid(),
                          position_event.get_vector());
      },
      position);
}
} // namespace details
} // namespace game