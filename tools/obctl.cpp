// obctl — replay de um fluxo de ordens em CSV contra o motor.
//
// Formato (uma operação por linha; '#' inicia comentário):
//   A,<id>,<B|S>,<LMT|MKT|IOC|FOK|POST>,<preço>,<qtd>   adiciona
//   C,<id>                                                cancela
//   M,<id>,<nova_qtd>[,<novo_preço>]                      modifica
//
// Saída: cada execução em CSV (T,taker,maker,lado,preço,qtd), rejeições
// (R,id,motivo) e, no fim, o topo do livro.
#include "orderbook/order_book.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

using namespace ob;

namespace {

std::vector<std::string> split(const std::string& s) {
  std::vector<std::string> out;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, ',')) out.push_back(item);
  return out;
}

bool parse_type(const std::string& t, OrderType& out) {
  if (t == "LMT") out = OrderType::Limit;
  else if (t == "MKT") out = OrderType::Market;
  else if (t == "IOC") out = OrderType::Ioc;
  else if (t == "FOK") out = OrderType::Fok;
  else if (t == "POST") out = OrderType::PostOnly;
  else return false;
  return true;
}

int usage() {
  std::fprintf(stderr, "uso: obctl <arquivo.csv | ->  [--depth N]\n");
  return 2;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) return usage();
  std::size_t depth_levels = 5;
  for (int i = 2; i + 1 < argc; ++i)
    if (std::string(argv[i]) == "--depth") depth_levels = std::stoul(argv[i + 1]);

  std::ifstream file;
  std::istream* in = &std::cin;
  if (std::string(argv[1]) != "-") {
    file.open(argv[1]);
    if (!file) { std::fprintf(stderr, "não abriu %s\n", argv[1]); return 1; }
    in = &file;
  }

  OrderBook book([](const Trade& t) {
    std::printf("T,%llu,%llu,%c,%lld,%llu\n", static_cast<unsigned long long>(t.taker_id),
                static_cast<unsigned long long>(t.maker_id), t.taker_side == Side::Buy ? 'B' : 'S',
                static_cast<long long>(t.price), static_cast<unsigned long long>(t.qty));
  });
  book.set_record_trades(false);

  std::string line;
  std::size_t lineno = 0, errors = 0;
  while (std::getline(*in, line)) {
    ++lineno;
    if (line.empty() || line[0] == '#') continue;
    if (!line.empty() && line.back() == '\r') line.pop_back();
    const auto f = split(line);
    try {
      if (f[0] == "A" && f.size() == 6) {
        Order o;
        o.id = std::stoull(f[1]);
        o.side = f[2] == "B" ? Side::Buy : Side::Sell;
        if (!parse_type(f[3], o.type)) throw std::invalid_argument("tipo");
        o.price = std::stoll(f[4]);
        o.qty = std::stoull(f[5]);
        auto r = book.add(o);
        if (!r.accepted()) std::printf("R,%s,%s\n", f[1].c_str(), to_string(r.reject));
      } else if (f[0] == "C" && f.size() == 2) {
        if (!book.cancel(std::stoull(f[1]))) std::printf("R,%s,unknown_order\n", f[1].c_str());
      } else if (f[0] == "M" && (f.size() == 3 || f.size() == 4)) {
        std::optional<Price> px;
        if (f.size() == 4) px = std::stoll(f[3]);
        auto r = book.modify(std::stoull(f[1]), std::stoull(f[2]), px);
        if (!r.accepted()) std::printf("R,%s,%s\n", f[1].c_str(), to_string(r.reject));
      } else {
        throw std::invalid_argument("formato");
      }
    } catch (const std::exception&) {
      ++errors;
      std::fprintf(stderr, "linha %zu inválida: %s\n", lineno, line.c_str());
    }
  }

  std::printf("# livro: %zu ordens\n", book.order_count());
  const auto asks = book.depth(Side::Sell, depth_levels);
  for (auto it = asks.rbegin(); it != asks.rend(); ++it)
    std::printf("# ASK %10lld  %10llu  (%u)\n", static_cast<long long>(it->price),
                static_cast<unsigned long long>(it->qty), it->orders);
  for (const auto& l : book.depth(Side::Buy, depth_levels))
    std::printf("# BID %10lld  %10llu  (%u)\n", static_cast<long long>(l.price),
                static_cast<unsigned long long>(l.qty), l.orders);
  return errors == 0 ? 0 : 1;
}
