# Métricas reais do simulador (geradas por gerar_figuras.py)

Correntes assumidas (placeholder de calibração): LÍDER=120.0 mA, MEMBRO=25.0 mA, IDLE=8.0 mA. 5 nós, seeds=[1, 2, 3, 4, 5], mandato=60 s, cooldown=120 s.

## Cenário A — homogêneo (5 nós idênticos, 100%)

| Política | FND médio (min) | Desvio (min) | vs Round-Robin |
|---|---|---|---|
| Round-Robin | 59.29 | 2.05 | — |
| Energia | 58.33 | 0.64 | -1.6% |
| Energia+Cooldown | 57.91 | 0.22 | -2.3% |

σ da liderança (menor = mais justo):

| Política | σ |
|---|---|
| Round-Robin | 0.656 |
| Energia | 0.796 |
| Energia+Cooldown | 0.891 |

Dispersão da energia residual no FND (menor = menos desperdício):

| Política | spread |
|---|---|
| Round-Robin | 2628.368 |
| Energia | 2720.106 |
| Energia+Cooldown | 3261.377 |

## Cenário B — carga inicial escalonada 100/85/70/55/40% (mesma capacidade)

| Política | FND médio (min) | Desvio (min) | vs Round-Robin |
|---|---|---|---|
| Round-Robin | 17.20 | 0.25 | — |
| Energia | 25.95 | 0.00 | +50.9% |
| Energia+Cooldown | 22.50 | 1.81 | +30.8% |

σ da liderança (menor = mais justo):

| Política | σ |
|---|---|
| Round-Robin | 0.721 |
| Energia | 5.996 |
| Energia+Cooldown | 3.536 |

Dispersão da energia residual no FND (menor = menos desperdício):

| Política | spread |
|---|---|
| Round-Robin | 2.257 |
| Energia | 0.346 |
| Energia+Cooldown | 1.246 |
