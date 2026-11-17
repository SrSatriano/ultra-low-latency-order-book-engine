#include "orderbook/order_book.hpp"

#include <algorithm>

namespace ob {

const char* to_string(Reject r) {
  switch (r) {
    case Reject::None: return "none";
    case Reject::DuplicateId: return "duplicate_id";
    case Reject::ZeroQuantity: return "zero_quantity";
    case Reject::InvalidPrice: return "invalid_price";
    case Reject::FokNotFillable: return "fok_not_fillable";
    case Reject::PostOnlyWouldCross: return "post_only_would_cross";
    case Reject::UnknownOrder: return "unknown_order";
  }
  return "?";
}

namespace {

// O preço do nível é aceitável para o agressor?
inline bool acceptable(Side taker, Price limit, Price level) {
  return taker == Side::Buy ? level <= limit : level >= limit;
}

}  // namespace

void OrderBook::emit(const Trade& t) {
  if (record_trades_) trades_.push_back(t);
  if (sink_) sink_(t);
}

template <class Book>
Qty OrderBook::fillable(const Book& book, const Order& taker, bool has_limit, Qty wanted) const {
  Qty total = 0;
  for (const auto& [price, level] : book) {
    if (has_limit && !acceptable(taker.side, taker.price, price)) break;
    total += level.total;
    if (total >= wanted) return total;
  }
  return total;
}

template <class Book>
Qty OrderBook::match(Book& book, const Order& taker, Qty remaining, bool has_limit) {
  Qty filled = 0;
  while (remaining > 0 && !book.empty()) {
    auto lvl_it = book.begin();
    const Price px = lvl_it->first;
    if (has_limit && !acceptable(taker.side, taker.price, px)) break;

    Level& level = lvl_it->second;
    while (remaining > 0 && !level.queue.empty()) {
      Resting& maker = level.queue.front();
      const Qty q = std::min(remaining, maker.qty);
      emit(Trade{taker.id, maker.id, taker.side, px, q});
      maker.qty -= q;
      level.total -= q;
      remaining -= q;
      filled += q;
      if (maker.qty == 0) {
        index_.erase(maker.id);
        level.queue.pop_front();
      }
    }
    if (level.queue.empty()) book.erase(lvl_it);
  }
  return filled;
}

void OrderBook::rest(const Order& order, Qty qty) {
  auto push = [&](auto& book) {
    Level& level = book[order.price];
    level.queue.push_back(Resting{order.id, qty});
    level.total += qty;
    index_.emplace(order.id, Handle{order.side, order.price, std::prev(level.queue.end())});
  };
  if (order.side == Side::Buy) push(bids_); else push(asks_);
}

bool OrderBook::crosses(Side side, Price price) const {
  if (side == Side::Buy) return !asks_.empty() && asks_.begin()->first <= price;
  return !bids_.empty() && bids_.begin()->first >= price;
}

AddResult OrderBook::add(const Order& order) {
  AddResult r;
  if (order.qty == 0) { r.reject = Reject::ZeroQuantity; return r; }
  if (index_.count(order.id)) { r.reject = Reject::DuplicateId; return r; }
  const bool has_limit = order.type != OrderType::Market;
  if (has_limit && order.price <= 0) { r.reject = Reject::InvalidPrice; return r; }

  if (order.type == OrderType::PostOnly && crosses(order.side, order.price)) {
    r.reject = Reject::PostOnlyWouldCross;
    return r;
  }
  if (order.type == OrderType::Fok) {
    const Qty available = order.side == Side::Buy
                              ? fillable(asks_, order, true, order.qty)
                              : fillable(bids_, order, true, order.qty);
    if (available < order.qty) { r.reject = Reject::FokNotFillable; return r; }
  }

  Qty remaining = order.qty;
  if (order.type != OrderType::PostOnly) {
    r.filled = order.side == Side::Buy ? match(asks_, order, remaining, has_limit)
                                       : match(bids_, order, remaining, has_limit);
    remaining -= r.filled;
  }

  const bool rests = order.type == OrderType::Limit || order.type == OrderType::PostOnly;
  if (remaining > 0 && rests) {
    rest(order, remaining);
    r.resting = remaining;
  }
  return r;
}

bool OrderBook::cancel(OrderId id) {
  auto it = index_.find(id);
  if (it == index_.end()) return false;
  const Handle h = it->second;
  auto drop = [&](auto& book) {
    auto lvl = book.find(h.price);
    lvl->second.total -= h.it->qty;
    lvl->second.queue.erase(h.it);
    if (lvl->second.queue.empty()) book.erase(lvl);
  };
  if (h.side == Side::Buy) drop(bids_); else drop(asks_);
  index_.erase(it);
  return true;
}

AddResult OrderBook::modify(OrderId id, Qty new_qty, std::optional<Price> new_price) {
  auto it = index_.find(id);
  if (it == index_.end()) return AddResult{Reject::UnknownOrder, 0, 0};
  Handle& h = it->second;
  if (new_qty == 0) { cancel(id); return AddResult{}; }

  const bool same_price = !new_price || *new_price == h.price;
  if (same_price && new_qty <= h.it->qty) {
    // Redução no lugar: mantém a posição na fila.
    auto shrink = [&](auto& book) {
      Level& lvl = book.find(h.price)->second;
      lvl.total -= h.it->qty - new_qty;
      h.it->qty = new_qty;
    };
    if (h.side == Side::Buy) shrink(bids_); else shrink(asks_);
    return AddResult{Reject::None, 0, new_qty};
  }

  const Order replacement{id, h.side, OrderType::Limit, new_price.value_or(h.price), new_qty};
  cancel(id);
  return add(replacement);
}

std::optional<Price> OrderBook::best_bid() const {
  if (bids_.empty()) return std::nullopt;
  return bids_.begin()->first;
}

std::optional<Price> OrderBook::best_ask() const {
  if (asks_.empty()) return std::nullopt;
  return asks_.begin()->first;
}

std::vector<PriceLevel> OrderBook::depth(Side side, std::size_t levels) const {
  std::vector<PriceLevel> out;
  auto collect = [&](const auto& book) {
    for (const auto& [price, level] : book) {
      if (out.size() == levels) break;
      out.push_back(PriceLevel{price, level.total, static_cast<uint32_t>(level.queue.size())});
    }
  };
  if (side == Side::Buy) collect(bids_); else collect(asks_);
  return out;
}

Qty OrderBook::volume_at(Side side, Price price) const {
  if (side == Side::Buy) {
    auto it = bids_.find(price);
    return it == bids_.end() ? 0 : it->second.total;
  }
  auto it = asks_.find(price);
  return it == asks_.end() ? 0 : it->second.total;
}

}  // namespace ob
