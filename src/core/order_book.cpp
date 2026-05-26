#include "orderbook/order_book.hpp"

#include <algorithm>

namespace ob {

void OrderBook::add_order(Order order) {
  auto& side = order.side == Side::Buy ? bids_ : asks_;
  auto it = std::lower_bound(
      side.begin(), side.end(), order.price_ticks,
      [&](const PriceLevel& lvl, int64_t px) {
        return order.side == Side::Buy ? lvl.price_ticks > px : lvl.price_ticks < px;
      });
  if (it != side.end() && it->price_ticks == order.price_ticks) {
    it->quantity += order.remaining;
  } else {
    side.insert(it, PriceLevel{order.price_ticks, order.remaining});
  }
  orders_by_id_[order.id] = order;
}

bool OrderBook::cancel(uint64_t order_id) {
  auto it = orders_by_id_.find(order_id);
  if (it == orders_by_id_.end()) return false;
  orders_by_id_.erase(it);
  return true;
}

}  // namespace ob
