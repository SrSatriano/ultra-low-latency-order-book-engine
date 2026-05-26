#include "orderbook/order_book.hpp"

#include <cassert>
#include <cstdio>

int main() {
  ob::OrderBook book;

  ob::Order sell{1, "s1", ob::Side::Sell, ob::OrderType::Limit, 100, 10, 10};
  ob::Order buy{2, "b1", ob::Side::Buy, ob::OrderType::Limit, 100, 8, 8};

  book.add_order(sell);
  assert(book.asks().size() == 1);
  assert(book.trades().empty());

  book.add_order(buy);
  assert(book.trades().size() == 1);
  assert(book.trades()[0].quantity == 8);
  assert(book.asks()[0].quantity == 2);

  assert(book.cancel(1));
  assert(book.asks().empty());

  ob::Order sell2{3, "s2", ob::Side::Sell, ob::OrderType::Limit, 105, 5, 5};
  ob::Order buy2{4, "b2", ob::Side::Buy, ob::OrderType::Limit, 110, 5, 5};
  book.add_order(sell2);
  book.add_order(buy2);
  assert(book.trades().size() == 2);

  std::puts("order_book_test: OK");
  return 0;
}
