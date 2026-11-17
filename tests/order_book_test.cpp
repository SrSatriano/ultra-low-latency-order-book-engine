#include "orderbook/order_book.hpp"

#include <cstdio>
#include <cstdlib>
#include <map>
#include <random>
#include <string>
#include <vector>

using namespace ob;

static int g_failed = 0;
static int g_checks = 0;

#define CHECK(cond)                                                         \
  do {                                                                      \
    ++g_checks;                                                             \
    if (!(cond)) {                                                          \
      ++g_failed;                                                           \
      std::fprintf(stderr, "  FALHOU %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    }                                                                       \
  } while (0)

#define TEST(name) static void name()
#define RUN(name)                         \
  do {                                    \
    std::printf("- %s\n", #name);         \
    name();                               \
  } while (0)

static Order lim(OrderId id, Side s, Price p, Qty q) { return {id, s, OrderType::Limit, p, q}; }

TEST(limit_rests_and_crosses_at_maker_price) {
  OrderBook b;
  CHECK(b.add(lim(1, Side::Sell, 101, 10)).resting == 10);
  auto r = b.add(lim(2, Side::Buy, 105, 4));
  CHECK(r.filled == 4 && r.resting == 0);
  CHECK(b.trades().size() == 1);
  CHECK(b.trades()[0].price == 101);  // preço do maker, não do agressor
  CHECK(b.trades()[0].maker_id == 1 && b.trades()[0].taker_id == 2);
  CHECK(b.volume_at(Side::Sell, 101) == 6);
}

TEST(price_time_priority_fifo) {
  OrderBook b;
  b.add(lim(1, Side::Sell, 100, 5));
  b.add(lim(2, Side::Sell, 100, 5));
  b.add(lim(3, Side::Sell, 99, 5));
  b.add(lim(4, Side::Buy, 100, 12));
  const auto& t = b.trades();
  CHECK(t.size() == 3);
  CHECK(t[0].maker_id == 3 && t[0].price == 99);  // melhor preço primeiro
  CHECK(t[1].maker_id == 1);                       // depois o mais antigo
  CHECK(t[2].maker_id == 2 && t[2].qty == 2);
  CHECK(b.volume_at(Side::Sell, 100) == 3);
}

TEST(market_sweeps_levels_and_never_rests) {
  OrderBook b;
  b.add(lim(1, Side::Buy, 100, 3));
  b.add(lim(2, Side::Buy, 99, 3));
  auto r = b.add({3, Side::Sell, OrderType::Market, 0, 10});
  CHECK(r.filled == 6 && r.resting == 0);
  CHECK(!b.best_bid().has_value());
  CHECK(!b.contains(3));
}

TEST(ioc_discards_remainder) {
  OrderBook b;
  b.add(lim(1, Side::Sell, 100, 3));
  b.add(lim(2, Side::Sell, 102, 3));
  auto r = b.add({3, Side::Buy, OrderType::Ioc, 101, 5});
  CHECK(r.filled == 3 && r.resting == 0);
  CHECK(!b.contains(3));
  CHECK(b.best_ask() == 102);
}

TEST(fok_all_or_nothing) {
  OrderBook b;
  b.add(lim(1, Side::Sell, 100, 3));
  b.add(lim(2, Side::Sell, 101, 3));
  auto no = b.add({3, Side::Buy, OrderType::Fok, 100, 5});
  CHECK(no.reject == Reject::FokNotFillable);
  CHECK(b.trades().empty());  // nada executa se não der para preencher tudo
  auto yes = b.add({4, Side::Buy, OrderType::Fok, 101, 5});
  CHECK(yes.accepted() && yes.filled == 5);
  CHECK(b.volume_at(Side::Sell, 101) == 1);
}

TEST(post_only_rejects_when_crossing) {
  OrderBook b;
  b.add(lim(1, Side::Sell, 100, 3));
  CHECK(b.add({2, Side::Buy, OrderType::PostOnly, 100, 1}).reject == Reject::PostOnlyWouldCross);
  auto ok = b.add({3, Side::Buy, OrderType::PostOnly, 99, 1});
  CHECK(ok.accepted() && ok.resting == 1 && b.trades().empty());
}

TEST(cancel_from_middle_of_queue) {
  OrderBook b;
  b.add(lim(1, Side::Buy, 100, 1));
  b.add(lim(2, Side::Buy, 100, 2));
  b.add(lim(3, Side::Buy, 100, 3));
  CHECK(b.cancel(2));
  CHECK(!b.cancel(2));
  CHECK(b.volume_at(Side::Buy, 100) == 4);
  auto d = b.depth(Side::Buy, 5);
  CHECK(d.size() == 1 && d[0].orders == 2);
  b.add(lim(4, Side::Sell, 100, 4));
  CHECK(b.trades()[0].maker_id == 1 && b.trades()[1].maker_id == 3);
  CHECK(b.order_count() == 0);
}

TEST(modify_reduce_keeps_priority_price_change_loses_it) {
  OrderBook b;
  b.add(lim(1, Side::Sell, 100, 10));
  b.add(lim(2, Side::Sell, 100, 10));
  CHECK(b.modify(1, 4).resting == 4);
  b.add(lim(3, Side::Buy, 100, 4));
  CHECK(b.trades().back().maker_id == 1);  // continuou na frente

  b.clear_trades();
  b.add(lim(5, Side::Sell, 100, 1));
  b.modify(2, 10, 100);  // mesmo preço e mesma qtd: mantém
  b.modify(5, 1, 99);    // novo preço: vira o melhor nível
  b.add(lim(6, Side::Buy, 100, 1));
  CHECK(b.trades().back().maker_id == 5 && b.trades().back().price == 99);
  CHECK(b.modify(42, 1).reject == Reject::UnknownOrder);
}

TEST(validations) {
  OrderBook b;
  CHECK(b.add(lim(1, Side::Buy, 100, 0)).reject == Reject::ZeroQuantity);
  CHECK(b.add(lim(1, Side::Buy, 0, 1)).reject == Reject::InvalidPrice);
  CHECK(b.add(lim(1, Side::Buy, 100, 1)).accepted());
  CHECK(b.add(lim(1, Side::Buy, 100, 1)).reject == Reject::DuplicateId);
  auto empty_mkt = b.add({2, Side::Buy, OrderType::Market, 0, 5});
  CHECK(empty_mkt.accepted() && empty_mkt.filled == 0 && empty_mkt.resting == 0);
}

TEST(depth_is_sorted_best_first) {
  OrderBook b;
  for (Price p : {98, 100, 99}) b.add(lim(static_cast<OrderId>(p), Side::Buy, p, 1));
  for (Price p : {103, 101, 102}) b.add(lim(static_cast<OrderId>(p), Side::Sell, p, 1));
  auto bids = b.depth(Side::Buy, 2);
  auto asks = b.depth(Side::Sell, 10);
  CHECK(bids.size() == 2 && bids[0].price == 100 && bids[1].price == 99);
  CHECK(asks.size() == 3 && asks[0].price == 101 && asks[2].price == 103);
}

// Compara o motor com um modelo de referência ingênuo (vetor ordenado) sob
// milhares de operações aleatórias e checa invariantes a cada passo.
TEST(randomized_against_reference_model) {
  struct Ref {
    std::vector<std::pair<OrderId, Order>> resting;  // ordem de chegada
    Qty match(Order o, std::vector<Trade>& out) {
      Qty filled = 0;
      while (o.qty > 0) {
        int best = -1;
        for (int i = 0; i < static_cast<int>(resting.size()); ++i) {
          const Order& m = resting[i].second;
          if (m.side == o.side) continue;
          const bool ok = o.type == OrderType::Market ||
                          (o.side == Side::Buy ? m.price <= o.price : m.price >= o.price);
          if (!ok) continue;
          if (best < 0) { best = i; continue; }
          const Order& bm = resting[best].second;
          const bool better = o.side == Side::Buy ? m.price < bm.price : m.price > bm.price;
          if (better) best = i;
        }
        if (best < 0) break;
        Order& m = resting[best].second;
        const Qty q = std::min(o.qty, m.qty);
        out.push_back({o.id, m.id, o.side, m.price, q});
        m.qty -= q; o.qty -= q; filled += q;
        if (m.qty == 0) resting.erase(resting.begin() + best);
      }
      if (o.qty > 0 && o.type == OrderType::Limit) resting.push_back({o.id, o});
      return filled;
    }
  } ref;

  std::mt19937_64 rng(12345);
  OrderBook b;
  std::vector<Trade> ref_trades;
  OrderId next = 1;
  for (int step = 0; step < 20000; ++step) {
    const int action = static_cast<int>(rng() % 10);
    if (action < 7) {
      Order o{next++, rng() % 2 ? Side::Buy : Side::Sell,
              rng() % 8 == 0 ? OrderType::Market : OrderType::Limit,
              static_cast<Price>(95 + rng() % 11), 1 + rng() % 20};
      b.add(o);
      ref.match(o, ref_trades);
    } else if (!ref.resting.empty()) {
      const auto idx = rng() % ref.resting.size();
      const OrderId id = ref.resting[idx].first;
      CHECK(b.cancel(id));
      ref.resting.erase(ref.resting.begin() + static_cast<long>(idx));
    }
    // Invariante: o livro nunca fica cruzado.
    if (b.best_bid() && b.best_ask()) CHECK(*b.best_bid() < *b.best_ask());
    if (g_failed) break;
  }
  CHECK(b.trades().size() == ref_trades.size());
  bool same = b.trades().size() == ref_trades.size();
  for (std::size_t i = 0; same && i < ref_trades.size(); ++i) {
    const auto& x = b.trades()[i];
    const auto& y = ref_trades[i];
    same = x.maker_id == y.maker_id && x.taker_id == y.taker_id && x.price == y.price &&
           x.qty == y.qty;
  }
  CHECK(same);
  CHECK(b.order_count() == ref.resting.size());
}

int main() {
  RUN(limit_rests_and_crosses_at_maker_price);
  RUN(price_time_priority_fifo);
  RUN(market_sweeps_levels_and_never_rests);
  RUN(ioc_discards_remainder);
  RUN(fok_all_or_nothing);
  RUN(post_only_rejects_when_crossing);
  RUN(cancel_from_middle_of_queue);
  RUN(modify_reduce_keeps_priority_price_change_loses_it);
  RUN(validations);
  RUN(depth_is_sorted_best_first);
  RUN(randomized_against_reference_model);
  std::printf("%d verificações, %d falhas\n", g_checks, g_failed);
  return g_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
