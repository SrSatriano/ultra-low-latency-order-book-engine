# Deployment Guide

## PrÃ©-requisitos

Consulte o README principal para dependÃªncias de runtime.

## HomologaÃ§Ã£o

```bash
# Exemplo genÃ©rico â€” adapte ao stack do projeto
cp .env.example .env
# instalar dependÃªncias conforme README
# executar migraÃ§Ãµes se houver banco
```

## ProduÃ§Ã£o

- VariÃ¡veis em secret manager (nunca no repositÃ³rio)
- Health check: endpoint documentado no README
- Backup e retenÃ§Ã£o conforme polÃ­tica da equipe

## Rollback

1. Reverter para tag `v1.0.0` anterior
2. Validar health checks
3. Reprocessar fila morta se aplicÃ¡vel
