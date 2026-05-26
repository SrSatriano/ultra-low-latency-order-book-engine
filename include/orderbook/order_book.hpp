#pragma once

#include "orderbook/types.hpp"

#include <unordered_map>
#include <vector>

namespace ob {

class OrderBook {
 public:
  void add_order(Order order);
  bool cancel(uint64_t order_id);

  const std::vector<PriceLevel>& bids() const { return bids_; }
  const std::vector<PriceLevel>& asks() const { return asks_; }

 private:
  std::vector<PriceLevel> bids_;
  std::vector<PriceLevel> asks_;
  std::unordered_map<uint64_t, Order> orders_by_id_;
};

}  // namespace ob
