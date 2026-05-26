#include "orderbook/order_book.hpp"

#include <chrono>
#include <cstdio>

int main() {
  ob::OrderBook book;
  constexpr std::size_t n = 500000;
  const auto t0 = std::chrono::steady_clock::now();
  for (std::size_t i = 0; i < n; ++i) {
    ob::Order o{i + 1,
                "c",
                (i % 2 == 0) ? ob::Side::Buy : ob::Side::Sell,
                ob::OrderType::Limit,
                static_cast<int64_t>(100 + (i % 10)),
                10,
                10};
    book.add_order(o);
  }
  const auto ms =
      std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0)
          .count();
  std::printf("orders=%zu time_ms=%.2f throughput=%.0f orders/s\n", n, ms,
              n / (ms / 1000.0));
  std::printf("bids=%zu asks=%zu\n", book.bids().size(), book.asks().size());
  return 0;
}
