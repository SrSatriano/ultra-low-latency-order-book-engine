# Architecture

## VisÃ£o geral

Este documento descreve a arquitetura em produÃ§Ã£o da versÃ£o **1.0.0**.

```mermaid
flowchart LR
  Client[Clients / Operators] --> API[Core Service]
  API --> Store[(Persistence)]
  API --> Metrics[Observability]
  Metrics --> Dashboard[Grafana / Logs]
```

## Componentes

| Componente | Responsabilidade |
|------------|------------------|
| Core | Regras de negÃ³cio e orquestraÃ§Ã£o |
| Persistence | Estado durÃ¡vel e idempotÃªncia |
| Observability | MÃ©tricas, traces e alertas |

## DecisÃµes de design

- **Baixa latÃªncia**: hot path sem alocaÃ§Ã£o desnecessÃ¡ria
- **Fail-safe**: degradaÃ§Ã£o graceful e reconciliaÃ§Ã£o
- **AuditÃ¡vel**: logs estruturados e rastreio de requisiÃ§Ãµes

## Escalabilidade

Escala horizontal no tier stateless; particionamento onde hÃ¡ estado (sÃ­mbolos, tenants, shards).
