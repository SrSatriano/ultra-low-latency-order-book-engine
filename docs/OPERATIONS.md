# Operations Runbook

## Health

- Liveness: processo respondendo
- Readiness: dependÃªncias (DB, RPC, fila) OK

## Alertas comuns

| Sintoma | AÃ§Ã£o |
|---------|------|
| LatÃªncia p99 alta | Verificar CPU throttling / GC / rede |
| Erro 5xx em rajada | Rollback + analisar logs |
| Fila crescendo | Escalar workers / verificar downstream |

## Backup

FrequÃªncia diÃ¡ria; teste de restore mensal documentado.
