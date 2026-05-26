# Guia de concorrência

## Lock-free SPSC

Produtor (gRPC) e consumidor (matcher) comunicam via ring buffer de potência de 2. Tamanho típico: 65536 slots.

## Evitar false sharing

Padding de 64 bytes entre contadores `head` e `tail` do ring buffer.

## Benchmarking justo

- Pin threads antes do warmup.
- Desabilitar turbo inconsistente: `cpufreq-set -g performance`.
- Repetir 5 runs; reportar p50, p99, p999.
