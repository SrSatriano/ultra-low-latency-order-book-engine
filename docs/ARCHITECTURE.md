# Arquitetura — Order Book Engine

## Componentes

1. **Ingest Gateway** — valida, normaliza timestamp (TSC → wall clock) e roteia para shard.
2. **Matching Engine** — price-time priority; suporta LIMIT, MARKET, IOC, FOK.
3. **Snapshot Service** — publica L2 a cada N ms ou N eventos via ZeroMQ.
4. **Recovery** — replay de WAL para reconstruir estado após crash.

## Modelo de memória

- Order book por símbolo: `std::vector<PriceLevel>` ordenado por preço.
- Índice auxiliar `unordered_map<OrderId, Location>` para cancel/replace O(1) amortizado.

## Threading

| Thread | Responsabilidade |
|--------|------------------|
| gRPC pool | Recebe ordens, enfileira |
| Matcher × N | Um ou mais shards |
| Publisher | Serializa snapshots ZMQ |

Isolamento de CPU recomendado: `taskset` ou `isolcpus` no Linux para threads de matching.
