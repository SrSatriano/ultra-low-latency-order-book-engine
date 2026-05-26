#include "orderbook/order_book.hpp"

#include <algorithm>

namespace ob {

namespace {

bool price_better_buy(int64_t incoming, int64_t level) { return incoming >= level; }
bool price_better_sell(int64_t incoming, int64_t level) { return incoming <= level; }

}  // namespace

void OrderBook::add_to_book(const Order& order) {
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
}

bool OrderBook::remove_quantity(Side side, int64_t price_ticks, uint64_t qty) {
  auto& book = side == Side::Buy ? bids_ : asks_;
  for (auto it = book.begin(); it != book.end(); ++it) {
    if (it->price_ticks != price_ticks) continue;
    if (it->quantity <= qty) {
      book.erase(it);
    } else {
      it->quantity -= qty;
    }
    return true;
  }
  return false;
}

void OrderBook::match_incoming(Order& order) {
  while (order.remaining > 0) {
    auto& contra = order.side == Side::Buy ? asks_ : bids_;
    if (contra.empty()) break;

    const auto& top = contra.front();
    const bool can_match = order.side == Side::Buy
                               ? price_better_buy(order.price_ticks, top.price_ticks)
                               : price_better_sell(order.price_ticks, top.price_ticks);
    if (order.type == OrderType::Limit && !can_match) break;

    const uint64_t fill_qty = std::min(order.remaining, top.quantity);
    const int64_t trade_px = top.price_ticks;

    Trade t{};
    if (order.side == Side::Buy) {
      t.buy_order_id = order.id;
      t.sell_order_id = 0;
    } else {
      t.sell_order_id = order.id;
      t.buy_order_id = 0;
    }
    t.price_ticks = trade_px;
    t.quantity = fill_qty;
    trades_.push_back(t);

    order.remaining -= fill_qty;
    if (top.quantity == fill_qty) {
      contra.erase(contra.begin());
    } else {
      contra.front().quantity -= fill_qty;
    }

    if (order.type == OrderType::Ioc || order.type == OrderType::Fok) break;
  }
}

void OrderBook::add_order(Order order) {
  if (order.type == OrderType::Market) {
    order.price_ticks = order.side == Side::Buy ? INT64_MAX : 0;
  }
  match_incoming(order);
  if (order.remaining > 0 && order.type != OrderType::Ioc && order.type != OrderType::Fok) {
    add_to_book(order);
    orders_by_id_[order.id] = order;
  }
}

bool OrderBook::cancel(uint64_t order_id) {
  auto it = orders_by_id_.find(order_id);
  if (it == orders_by_id_.end()) return false;
  const Order& o = it->second;
  remove_quantity(o.side, o.price_ticks, o.remaining);
  orders_by_id_.erase(it);
  return true;
}

}  // namespace ob
