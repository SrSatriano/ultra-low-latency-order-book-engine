# Arquitetura

## Estruturas

```
OrderBook
├── bids_  std::map<Price, Level, greater>   melhor compra = begin()
├── asks_  std::map<Price, Level, less>      melhor venda  = begin()
│     Level { total, std::list<Resting> queue }   fila FIFO do nível
└── index_ std::unordered_map<OrderId, Handle>
      Handle { lado, preço, iterador para o nó na fila }
```

- **Inserção de ordem que descansa:** O(log N) no mapa de níveis, mais um `push_back` na fila.
- **Cancelamento:** lookup no hash, mais `erase` do nó pelo iterador, que é O(1). O nível é removido quando fica vazio.
- **Matching:** consome do `begin()` do lado contrário enquanto o preço for aceitável. Cada nó consumido sai do índice.
- **FOK:** antes de executar, soma a liquidez disponível até o preço-limite e, se não bastar, rejeita sem tocar no livro.
- **Modificação:** a redução de quantidade é feita no lugar e mantém a prioridade. As demais mudanças fazem cancelamento e reinserção.

## Fluxo de uma ordem

```
add(order)
  ├─ valida: qtd > 0, id único, preço > 0 (exceto Market)
  ├─ PostOnly cruzaria?  → rejeita
  ├─ FOK sem liquidez?   → rejeita
  ├─ match() contra o lado oposto (gera Trade por maker tocado)
  └─ sobra e é Limit/PostOnly? → rest() no fim da fila do nível
```

As execuções vão para um `TradeSink` (callback) e, opcionalmente, para um histórico interno. O histórico pode ser desligado em replays longos.

## Correção

O teste `randomized_against_reference_model` implementa o mesmo livro como um vetor simples, com busca linear do melhor preço e desempate pela ordem de chegada. Em seguida, compara execução por execução com o motor durante 20 mil operações aleatórias, com seed fixa, e checa a cada passo que o livro não fica cruzado.
