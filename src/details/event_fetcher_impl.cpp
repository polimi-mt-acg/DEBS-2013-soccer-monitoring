#include "details/event_fetcher_impl.hpp"
#include "event_fetcher.hpp"

namespace game {
namespace details {
event_fetcher_iterator::event_fetcher_iterator(EventFetcher &f, bool set_end)
    : fetcher{std::addressof(f)}, value{fetcher->batch, false,
                                        fetcher->context.take_snapshot()} {
  // Set to end if set_end == true or if fetcher input stream failbit is set
  // (i.e. EOF may be reached)
  is_end = set_end ? true : !(*fetcher->is);
}

event_fetcher_iterator::iterator &event_fetcher_iterator::operator++() {
  value = fetcher->parse_batch();
  is_end = !(*fetcher->is);
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
} // namespace details
} // namespace game