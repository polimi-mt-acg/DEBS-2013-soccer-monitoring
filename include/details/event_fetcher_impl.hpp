#ifndef SOCCER_MONITORING_EVENT_FETCHER_IMPL_HPP
#define SOCCER_MONITORING_EVENT_FETCHER_IMPL_HPP

#include <functional>
#include <utility>
#include <vector>

#include "batch.hpp"
#include "event.hpp"

namespace game {
class EventFetcher; // Forward-declared
class Batch;

namespace details {
class event_fetcher_iterator {
public:
  using difference_type = std::ptrdiff_t;
  using value_type = Batch;
  using reference = const value_type &;
  using pointer = std::add_pointer_t<reference>;
  using iterator_category = std::input_iterator_tag;

  using iterator = event_fetcher_iterator;

  event_fetcher_iterator(EventFetcher &f, bool set_end = false);

  // CopyConstructible
  event_fetcher_iterator(iterator const &other)
      : fetcher{other.fetcher}, is_end{other.is_end}, value{other.value} {}

  // CopyAssignable
  iterator &operator=(iterator const &other);

  // Dereferenceable (convertible to value_type)
  reference operator*() { return value; }
  value_type operator*() const { return value; }

  pointer operator->() const { return &value; }

  iterator &operator++();
  iterator operator++(int);

  friend bool operator==(iterator const &lhs, iterator const &rhs) {
    return lhs.is_end == rhs.is_end;
  }

  friend bool operator!=(iterator const &lhs, iterator const &rhs) {
    return !(lhs == rhs);
  }

private:
  EventFetcher *fetcher;
  Batch value;
  bool is_end;
};
} // namespace details
} // namespace game

#endif // SOCCER_MONITORING_EVENT_FETCHER_IMPL_HPP
