#pragma once

#include "orderbook/types.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace ob {

struct Trade {
  uint64_t buy_order_id{};
  uint64_t sell_order_id{};
  int64_t price_ticks{};
  uint64_t quantity{};
};

class OrderBook {
 public:
  void add_order(Order order);
  bool cancel(uint64_t order_id);

  const std::vector<PriceLevel>& bids() const { return bids_; }
  const std::vector<PriceLevel>& asks() const { return asks_; }
  const std::vector<Trade>& trades() const { return trades_; }

 private:
  void match_incoming(Order& order);
  void add_to_book(const Order& order);
  bool remove_quantity(Side side, int64_t price_ticks, uint64_t qty);

  std::vector<PriceLevel> bids_;
  std::vector<PriceLevel> asks_;
  std::unordered_map<uint64_t, Order> orders_by_id_;
  std::vector<Trade> trades_;
};

}  // namespace ob
