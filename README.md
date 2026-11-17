# Ultra-Low Latency Order Book Engine

[![CI](https://github.com/SrSatriano/ultra-low-latency-order-book-engine/actions/workflows/ci.yml/badge.svg)](https://github.com/SrSatriano/ultra-low-latency-order-book-engine/actions/workflows/ci.yml)
![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?logo=cplusplus)
![licença](https://img.shields.io/badge/licen%C3%A7a-MIT-22c55e)

Motor de matching de livro de ofertas em C++20, com prioridade preço-tempo, cinco tipos de ordem, replay de sessões em CSV e benchmark de latência por operação. Sem dependências além da biblioteca padrão.

## O que ele faz

- **Prioridade preço-tempo:** melhor preço primeiro e, no mesmo preço, a ordem mais antiga. O agressor sempre executa ao preço do maker.
- **Tipos de ordem:**
  - **Limit:** o que não executar descansa no livro.
  - **Market:** executa contra o que houver.
  - **IOC:** executa até o preço-limite e descarta o resto.
  - **FOK:** executa tudo ou nada, e a liquidez é conferida antes de tocar no livro.
  - **Post-only:** é rejeitada se fosse cruzar o livro.
- **Cancelamento O(1):** um índice por id aponta direto para o nó da fila do nível.
- **Modificação com regra de bolsa:** reduzir a quantidade mantém a posição na fila; mudar o preço ou aumentar a quantidade manda a ordem para o fim.
- **Consultas:** melhor compra e venda, profundidade agregada por nível (com nº de ordens) e volume em um preço.
- **Validações:** id duplicado, quantidade zero e preço inválido viram rejeição, nunca exceção.

## Números

Benchmark com 2 milhões de operações sintéticas (65% limit perto do topo, 25% cancelamentos, 10% agressões IOC), em um núcleo, build Release com GCC 13:

| Métrica | Valor |
|---|---|
| Vazão | ~2,3 milhões de ops/s |
| Latência p50 | ~200 ns |
| Latência p99 | ~1,5 µs |
| Latência p99,9 | ~5 µs |

A latência inclui a leitura do relógio em cada operação. Rode `./build/bench_orderbook` para medir na sua máquina.

## Como usar

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/bench_orderbook 2000000
./build/obctl examples/sessao.csv
```

### Replay em CSV

`obctl` lê um fluxo de ordens e imprime cada execução e rejeição, e o topo do livro no fim:

```
A,<id>,<B|S>,<LMT|MKT|IOC|FOK|POST>,<preço em ticks>,<qtd>
C,<id>
M,<id>,<nova qtd>[,<novo preço>]
```

```
$ ./build/obctl examples/sessao.csv
T,6,1,B,10100,50          # taker 6 comprou 50 do maker 1 a 101,00
T,6,2,B,10100,10
R,7,post_only_would_cross
R,8,fok_not_fillable
T,9,4,S,9900,25
T,9,5,S,9800,45
...
# ASK      10100          20  (1)
# BID       9850          30  (1)
```

### Como biblioteca

```cpp
#include "orderbook/order_book.hpp"

ob::OrderBook book([](const ob::Trade& t) { /* publica a execução */ });
book.add({1, ob::Side::Sell, ob::OrderType::Limit, 10100, 50});
auto r = book.add({2, ob::Side::Buy, ob::OrderType::Ioc, 10100, 80});
// r.filled == 50, r.resting == 0
```

## Testes

- 11 cenários de regra de negócio: FIFO, varredura de níveis, IOC, FOK, post-only, cancelamento no meio da fila, prioridade na modificação e validações.
- Um **teste de propriedade** que roda 20 mil operações aleatórias contra um modelo de referência ingênuo e exige as mesmas execuções, na mesma ordem, além de nunca deixar o livro cruzado.
- Um **teste golden** do `obctl`, que compara a saída do replay com `examples/sessao.expected`.
- O CI compila com GCC e Clang (com AddressSanitizer e UBSan) e com MSVC no Windows.

## Arquitetura

Veja [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## Limitações

- Thread única por livro. O modelo pensado é um livro por instrumento, cada um na sua thread.
- Não há rede, persistência nem protocolo de bolsa (FIX/ITCH). A entrada é a API C++ ou o replay CSV.
- `std::list` e `std::map` simplificam o código, mas alocam a cada inserção. Um pool de nós e níveis em array são o próximo passo para cortar a cauda de latência.

## Autor

Matheus Rodrigues Satriano — [github.com/SrSatriano](https://github.com/SrSatriano)
