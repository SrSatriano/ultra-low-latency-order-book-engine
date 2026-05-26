#include "orderbook/order_book.hpp"

#include <cassert>

int main() {
  ob::OrderBook book;
  ob::Order o{1, "c1", ob::Side::Buy, ob::OrderType::Limit, 100, 10, 10};
  book.add_order(o);
  assert(!book.bids().empty());
  assert(book.bids()[0].quantity == 10);
  assert(book.cancel(1));
  return 0;
}
