#pragma once

#include "orderbook/types.hpp"

#include <functional>
#include <list>
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

namespace ob {

// Livro de ofertas com prioridade preço-tempo (FIFO por nível).
//
// - Níveis ficam em mapas ordenados (compra: maior preço primeiro; venda: menor).
// - Cada nível guarda uma fila FIFO de ordens; o índice por id aponta direto
//   para o nó da fila, então cancelar é O(1) depois do lookup no hash.
// - O agressor executa ao preço do maker (o nível que já estava no livro).
class OrderBook {
 public:
  using TradeSink = std::function<void(const Trade&)>;

  OrderBook() = default;
  explicit OrderBook(TradeSink sink) : sink_(std::move(sink)) {}

  AddResult add(const Order& order);
  bool cancel(OrderId id);
  // Reduzir a quantidade mantém a prioridade; mudar o preço ou aumentar a
  // quantidade reenvia a ordem e ela vai para o fim da fila.
  AddResult modify(OrderId id, Qty new_qty, std::optional<Price> new_price = std::nullopt);

  std::optional<Price> best_bid() const;
  std::optional<Price> best_ask() const;
  std::vector<PriceLevel> depth(Side side, std::size_t levels) const;
  Qty volume_at(Side side, Price price) const;
  std::size_t order_count() const { return index_.size(); }
  bool contains(OrderId id) const { return index_.count(id) != 0; }

  const std::vector<Trade>& trades() const { return trades_; }
  void clear_trades() { trades_.clear(); }
  // Desligar o histórico interno evita crescer memória em replays longos;
  // o TradeSink continua recebendo cada execução.
  void set_record_trades(bool on) { record_trades_ = on; }

 private:
  struct Resting {
    OrderId id;
    Qty qty;
  };
  struct Level {
    Qty total{};
    std::list<Resting> queue;
  };
  using Bids = std::map<Price, Level, std::greater<Price>>;
  using Asks = std::map<Price, Level, std::less<Price>>;
  struct Handle {
    Side side;
    Price price;
    std::list<Resting>::iterator it;
  };

  template <class Book>
  Qty match(Book& book, const Order& taker, Qty remaining, bool has_limit);
  template <class Book>
  Qty fillable(const Book& book, const Order& taker, bool has_limit, Qty wanted) const;
  void rest(const Order& order, Qty qty);
  bool crosses(Side side, Price price) const;
  void emit(const Trade& t);

  Bids bids_;
  Asks asks_;
  std::unordered_map<OrderId, Handle> index_;
  std::vector<Trade> trades_;
  TradeSink sink_;
  bool record_trades_{true};
};

}  // namespace ob
