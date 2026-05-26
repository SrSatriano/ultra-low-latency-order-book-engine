# Ultra-Low Latency Order Book Engine

Motor de livro de ofertas de altíssima performance para simulação de exchanges. Foco em otimização de memória e execução paralela para latência na casa dos microssegundos.

## Stack

- **C++20** — núcleo de matching
- **gRPC** — API de ingestão de ordens
- **ZeroMQ** — pub/sub de snapshots e deltas do book

## Arquitetura de concorrência

```
                    ┌─────────────────┐
  gRPC Clients ───► │ Ingest Gateway  │
                    └────────┬────────┘
                             │ lock-free queue (SPSC)
                    ┌────────▼────────┐
                    │ Matching Engine │◄── shard por símbolo
                    │  (per-symbol)   │
                    └────────┬────────┘
                             │
              ┌──────────────┼──────────────┐
              ▼              ▼              ▼
         ZMQ Publisher   Audit Log    Metrics (p99)
```

- **Sharding por instrumento**: cada par tem uma fila dedicada; evita contenção global.
- **Estruturas cache-friendly**: níveis de preço em arrays contíguos; heap apenas para ordens GTC fora do top-of-book.
- **Sem alocação no hot path**: pools pré-alocados para `Order` e eventos de trade.

Documentação detalhada: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | [docs/CONCURRENCY.md](docs/CONCURRENCY.md)

## Benchmarks de latência

| Métrica | Alvo (release) | Ambiente de referência |
|---------|----------------|------------------------|
| Ingest → ACK | < 5 µs (p50) | AMD Ryzen 9, `-O3 -march=native` |
| Ingest → ACK | < 15 µs (p99) | idem |
| Throughput | > 2M ordens/s agregado | 8 shards, 1 símbolo/shard |

Executar benchmarks:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_BENCHMARKS=ON
cmake --build build --parallel
./build/benchmarks/order_book_throughput --symbols=8 --duration=60s
```

## Compilação com flags de otimização

```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_FLAGS="-O3 -march=native -flto -fno-exceptions" \
  -DUSE_TBB=ON

cmake --build build --parallel $(nproc)
```

### Multicore avançado

- **`-march=native`**: habilita AVX2/AVX-512 conforme CPU.
- **Link-Time Optimization (`-flto`)**: inlining cross-TU no matching loop.
- **Intel TBB** (opcional): work-stealing para replay histórico e rebuild de snapshots.

## Estrutura do repositório

| Pasta | Descrição |
|-------|-----------|
| `src/core/` | Tipos de ordem, price levels, book |
| `src/matching/` | Engine FIFO por preço |
| `src/network/` | Adaptadores gRPC e ZeroMQ |
| `benchmarks/` | Harness de latência e throughput |
| `proto/` | Definições `.proto` |
| `docs/` | Arquitetura, tuning, guias |

## Quick start

```bash
# Dependências: protobuf, grpc, libzmq, cmake >= 3.20
./scripts/build.sh
./build/order_book_server --config config/default.yaml
```

## Licença

MIT — uso educacional e de pesquisa. Não constitui software de exchange regulada.
