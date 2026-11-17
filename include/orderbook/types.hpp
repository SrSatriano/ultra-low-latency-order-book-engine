#pragma once

#include <cstdint>

namespace ob {

using OrderId = uint64_t;
using Price = int64_t;  // em ticks (ex.: centavos)
using Qty = uint64_t;

enum class Side : uint8_t { Buy, Sell };

// Limit:    descansa no livro o que não executar (GTC).
// Market:   executa contra o que houver; o resto é descartado.
// Ioc:      executa até o preço-limite; o resto é descartado.
// Fok:      executa tudo até o preço-limite ou nada.
// PostOnly: só entra se não cruzar o livro (garante posição de maker).
enum class OrderType : uint8_t { Limit, Market, Ioc, Fok, PostOnly };

struct Order {
  OrderId id{};
  Side side{Side::Buy};
  OrderType type{OrderType::Limit};
  Price price{};
  Qty qty{};
};

struct Trade {
  OrderId taker_id{};
  OrderId maker_id{};
  Side taker_side{Side::Buy};
  Price price{};
  Qty qty{};
};

struct PriceLevel {
  Price price{};
  Qty qty{};
  uint32_t orders{};
};

enum class Reject : uint8_t {
  None,
  DuplicateId,
  ZeroQuantity,
  InvalidPrice,
  FokNotFillable,
  PostOnlyWouldCross,
  UnknownOrder,
};

struct AddResult {
  Reject reject{Reject::None};
  Qty filled{};
  Qty resting{};
  bool accepted() const { return reject == Reject::None; }
};

const char* to_string(Reject r);

}  // namespace ob
