# Operações e runbook — Ultra-Low Latency Order Book Engine

> Versão **1.0.0**

## 1. Escopo

Procedimentos para equipes de **SRE**, **devops** ou **plantão** operarem o serviço em produção.

## 2. Monitoramento

| Métrica | Limiar sugerido | Ação |
|---------|-----------------|------|
| Taxa de erro HTTP/RPC | > 1% por 5 min | Investigar logs; considerar rollback |
| Latência p99 | 2× baseline | Verificar dependências e saturação |
| Uso de disco | > 85% | Expandir volume ou rotacionar logs |
| Fila de jobs | Crescimento contínuo | Escalar workers |

## 3. Logs

- Formato recomendado: **JSON estruturado** com `timestamp`, `level`, `trace_id`, `message`.
- Níveis: `DEBUG` apenas em homologação; `INFO`/`WARN`/`ERROR` em produção.
- Retenção: alinhar à política de compliance (ex.: 30–90 dias).

## 4. Alertas

Configure alertas para:

1. Health check falhando por 2 ciclos consecutivos.
2. Pico de latência ou erro após deploy (comparar com janela anterior).
3. Certificado TLS a expirar em < 14 dias.

## 5. Incidentes comuns

### 5.1 Serviço não sobe

1. Verificar variáveis de ambiente obrigatórias.
2. Conferir porta em uso e permissões.
3. Ler últimas 200 linhas de log de boot.

### 5.2 Dependência externa indisponível

1. Ativar modo degradado se existir (ver README).
2. Aumentar timeout temporariamente **somente** com circuit breaker.
3. Comunicar stakeholders conforme SLA.

### 5.3 Performance degradada

1. Comparar deploy recente vs baseline.
2. Verificar CPU, memória, GPU e I/O.
3. Reproduzir benchmark documentado no README.

## 6. Manutenção programada

- Janela preferencial: fora do horário de pico do negócio.
- Comunicar usuários com antecedência mínima de 24 h.
- Executar backup antes de migrations de schema.

## 7. Contatos e segurança

- Vulnerabilidades: [SECURITY.md](../SECURITY.md)
- Contribuições: [CONTRIBUTING.md](../CONTRIBUTING.md)

## 8. Procedimento pós-incidente

1. Timeline factual (UTC-3).
2. Causa raiz e ações corretivas.
3. Atualizar este runbook se houve lacuna de documentação.
