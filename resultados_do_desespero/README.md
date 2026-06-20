# resultados_do_desespero

Reescrita da **Seção 5.1 (Simulação)** do artigo, com um conjunto enxuto de
figuras geradas a partir do **simulador fiel ao firmware** (`analysis/`). Tudo
aqui é reproduzível e honesto: nenhum número foi digitado à mão e nenhum dado foi
fabricado.

## Procedência (importante)

Estes resultados vêm da **simulação numérica**, não de medição em hardware. O
simulador (`analysis/simulator/`) é um *port* passo a passo do *firmware* (loop de
100 ms, eleição/rotação, *cooldown*, contabilidade de energia), com parâmetros
sincronizados (`analysis/params.py`) e conformância testada contra o
`leader_policy.cpp` real.

**Premissa-chave:** o modelo assume que liderar custa mais que ser membro
(referência 120 mA vs. 25 mA — `analysis/calibration.py`, ainda *placeholder* de
calibração). É essa diferença que dá às políticas de energia margem para
redistribuir desgaste. A medição em hardware (Seção 5.2 do artigo) mostrou que, na
arquitetura adotada (rádio sempre ativo), líder ≈ membro em consumo — por isso o
ganho previsto pela simulação **não** aparece na bancada. A reescrita explicita
essa divergência em vez de escondê-la.

## Cenários (espelham os ensaios de laboratório, 5 nós)

- **Cenário A — carga plena:** os 5 nós começam com 100 %. As três políticas
  **empatam** em tempo de vida (≈ 58–59 min, dentro de ~2 %).
- **Cenário B — carga inicial escalonada:** mesma capacidade, cargas iniciais de
  100/85/70/55/40 % (≈ 40 % a 100 %). A seleção por energia residual **estende o
  FND em ~51 %** vs. Round-Robin (+31 % com *cooldown*), ao custo de concentrar a
  liderança (trade-off durabilidade × justiça).

  > Esses níveis são **exatamente** os do firmware usado em bancada
  > (`main/src/Application/reset_service.cpp` → `STAGGER_LEVELS = {100, 85, 70, 55,
  > 40}`, atribuídos por ordem de MAC; ver também `server/grafana/roteiro-ensaios.md`).
  > A simulação e os ensaios usam, portanto, o mesmo escalonamento.

## Conteúdo

| Arquivo | O que é |
|---|---|
| `secao_5.1_reescrita.md` | Texto que substitui a Seção 5.1 (cole no artigo). |
| `metricas.md` | Tabelas com os números reais que sustentam o texto. |
| `gerar_figuras.py` | Script que regenera figuras e métricas. |
| `fig_A_fnd_homogeneo.{png,pdf}` | FND por política, Cenário A (empate). |
| `fig_A_deplecao_homogeneo.{png,pdf}` | Curvas de depleção, Cenário A. |
| `fig_B_tradeoff_escalonado.{png,pdf}` | Durabilidade × justiça, Cenário B (+51 %/+31 %). |
| `fig_B_deplecao_escalonado.{png,pdf}` | Curvas de depleção, Cenário B (mecanismo do ganho). |

## Reproduzir

Da raiz do repositório:

```bash
pip install -r analysis/requirements.txt
python resultados_do_desespero/gerar_figuras.py
```

Saem os PNG/PDF nesta pasta e o `metricas.md` atualizado.

## O que NÃO está aqui

As subseções antigas **5.1.2 (Uplink Wi-Fi, Fig. 6–7)** e **5.1.3 (Plano de
Controle, Fig. 8–10)** não são reproduzíveis a partir deste simulador (ele não
modela PDR de *uplink* nem métricas de falha de eleição/retry/dual-leader). Foram
**removidas** nesta consolidação para não apresentar figuras que não conseguimos
regenerar. Se quiser mantê-las, reaproveite o texto/figuras da versão anterior do
artigo — mas confira a procedência delas antes.
