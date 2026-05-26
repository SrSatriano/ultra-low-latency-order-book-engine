# Política de segurança

## Versões com suporte

| Versão | Suporte |
|--------|---------|
| **1.0.x** | Ativo — correções de segurança |
| < 1.0 | Sem suporte |

## Reportar uma vulnerabilidade

**Não** abra issues públicas para vulnerabilidades.

1. Envie descrição detalhada para o mantenedor via GitHub Security Advisories ou contato privado do repositório [ultra-low-latency-order-book-engine](https://github.com/SrSatriano/ultra-low-latency-order-book-engine).
2. Inclua passos para reproduzir, impacto e versão afetada.
3. Aguarde confirmação antes de divulgação coordenada.

Objetivo de resposta inicial: **72 horas úteis**.

## Boas práticas para usuários

- Rotacione chaves de API periodicamente.
- Use `.env` local e nunca faça commit de segredos.
- Em projetos Web3, valide contratos em testnet antes de mainnet.
- Em ferramentas educacionais (ex.: MEV), **não** use com fundos reais em mainnet.

## Dependências

- Execute auditoria do ecossistema (`npm audit`, `pip audit`, `cargo audit`) antes de releases.
- Atualize dependências com CVE conhecidos em patches 1.0.x.
