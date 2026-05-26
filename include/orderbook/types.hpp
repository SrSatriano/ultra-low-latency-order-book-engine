#pragma once

#include <cstdint>
#include <string>

namespace ob {

enum class Side : uint8_t { Buy, Sell };
enum class OrderType : uint8_t { Limit, Market, Ioc, Fok };

struct Order {
  uint64_t id{};
  std::string client_order_id;
  Side side{Side::Buy};
  OrderType type{OrderType::Limit};
  int64_t price_ticks{};
  uint64_t quantity{};
  uint64_t remaining{};
};

struct PriceLevel {
  int64_t price_ticks{};
  uint64_t quantity{};
};

}  // namespace ob
