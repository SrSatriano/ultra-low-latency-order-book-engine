# Changelog

Todas as mudanças notáveis deste projeto são documentadas neste arquivo.

Formato baseado em [Keep a Changelog](https://keepachangelog.com/pt-BR/1.0.0/).

## [1.1.0] — 2026-11-17

### Mudado

- Motor reescrito com prioridade preço-tempo de verdade (fila FIFO por nível) e índice por id para cancelamento O(1).
- Execuções agora registram maker e taker.

### Adicionado

- Ordens IOC, FOK (tudo ou nada, checado antes de executar) e post-only.
- Modificação de ordem com regra de prioridade de bolsa.
- Consultas de profundidade, melhor preço e volume por nível.
- `obctl`: replay de sessões em CSV.
- `bench_orderbook`: vazão e latência p50/p99/p99.9.
- Teste de propriedade contra modelo de referência e teste golden do replay.
- CI com GCC, Clang (ASan/UBSan) e MSVC.

### Removido

- Arquivos de configuração, contrato gRPC e documentos de deploy que descreviam recursos não implementados.

## [1.0.0] — 2026-03-26

### Adicionado

- Documentação completa em português brasileiro (README e `docs/`)
- Implementação do núcleo descrito no README
- Suíte de testes: `ctest --test-dir build`
- Pipeline CI em `.github/workflows/ci.yml`

### Desempenho

- Benchmarks documentados no README e em `docs/ARCHITECTURE.md`

### Segurança

- Política em [SECURITY.md](SECURITY.md)
- Revisão de dependências na release 1.0.0

## [0.9.0] — 2026-02-15

### Adicionado

- Release candidate com contratos de API estáveis
- Testes de carga em homologação

## [0.1.0] — 2026-01-10

### Adicionado

- Estrutura inicial do repositório e prova de conceito
