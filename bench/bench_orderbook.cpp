// Benchmark: fluxo sintético parecido com o de uma bolsa (maioria limit perto
// do topo, cancelamentos frequentes, algumas agressões). Mede vazão e a
// latência de cada operação (p50/p99/p99.9).
#include "orderbook/order_book.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

using namespace ob;
using Clock = std::chrono::steady_clock;

int main(int argc, char** argv) {
  const std::size_t n = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1'000'000;
  std::mt19937_64 rng(42);
  std::vector<OrderId> live;
  live.reserve(n);

  Qty traded = 0;
  OrderBook counted([&](const Trade& t) { traded += t.qty; });
  counted.set_record_trades(false);

  std::vector<uint32_t> lat_ns;
  lat_ns.reserve(n);
  OrderId next = 1;
  const auto t0 = Clock::now();
  for (std::size_t i = 0; i < n; ++i) {
    const auto roll = rng() % 100;
    const auto s = Clock::now();
    if (roll < 25 && !live.empty()) {
      const auto k = rng() % live.size();
      counted.cancel(live[k]);
      live[k] = live.back();
      live.pop_back();
    } else {
      const Side side = rng() % 2 ? Side::Buy : Side::Sell;
      const bool aggressive = roll >= 90;
      const Price mid = 10'000;
      const Price offset = static_cast<Price>(rng() % 50);
      Price px = side == Side::Buy ? mid - 1 - offset : mid + 1 + offset;
      if (aggressive) px = side == Side::Buy ? mid + 60 : mid - 60;
      const Order o{next++, side, aggressive ? OrderType::Ioc : OrderType::Limit, px,
                    1 + rng() % 100};
      if (counted.add(o).resting > 0) live.push_back(o.id);
    }
    lat_ns.push_back(static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - s).count()));
  }
  const double secs = std::chrono::duration<double>(Clock::now() - t0).count();

  std::sort(lat_ns.begin(), lat_ns.end());
  auto pct = [&](double p) { return lat_ns[static_cast<std::size_t>(p * (lat_ns.size() - 1))]; };
  std::printf("operações       %zu\n", n);
  std::printf("vazão           %.2f milhões de ops/s\n", n / secs / 1e6);
  std::printf("latência p50    %u ns\n", pct(0.50));
  std::printf("latência p99    %u ns\n", pct(0.99));
  std::printf("latência p99.9  %u ns\n", pct(0.999));
  std::printf("ordens no livro %zu, volume executado %llu\n", counted.order_count(),
              static_cast<unsigned long long>(traded));
  return 0;
}
