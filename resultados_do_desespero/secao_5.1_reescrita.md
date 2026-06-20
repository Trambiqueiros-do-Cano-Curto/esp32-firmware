# 5.1 Simulação — texto de substituição

> **Como usar este arquivo.** O texto abaixo substitui integralmente a Seção 5.1
> (subseções 5.1.1, 5.1.2 e 5.1.3) da versão atual do artigo. Ele consolida a
> análise de simulação nos dois cenários dos ensaios de laboratório (5 nós) e em
> torno dos resultados que o simulador fiel ao firmware (`analysis/`) reproduz de
> ponta a ponta: tempo de vida (FND), energia residual e balanceamento da
> liderança. As figuras citadas estão nesta pasta e são geradas por
> `gerar_figuras.py`; **os números conferem com `metricas.md`** (nada foi digitado
> à mão). As subseções antigas de *Uplink Wi-Fi* (Fig. 6–7) e *Plano de Controle*
> (Fig. 8–10) **não** são reproduzíveis a partir deste simulador e por isso foram
> retiradas desta versão; se quiser preservá-las, reaproveite o texto da versão
> anterior, conferindo a procedência das figuras.

---

## 5.1 Simulação

A avaliação por simulação compara as três políticas de rotação de liderança —
*Round-Robin*, energia residual e energia residual com *cooldown* — nos mesmos
dois cenários usados em bancada: carga inicial plena (Cenário A) e carga inicial
escalonada (Cenário B). O simulador opera em passo discreto de 100 ms, espelhando
o ciclo principal do *firmware*, e mantém sincronizados os parâmetros de
descoberta, intervalo de leitura, duração do mandato (60 s) e janela de *cooldown*
(120 s). O *cluster* simulado tem **cinco nós**, como nos ensaios, e cada ponto
reportado é a média de cinco execuções independentes (sementes distintas).

Um ponto metodológico é central para interpretar os resultados: no modelo
energético, o papel de **líder** consome corrente substancialmente maior que o de
**membro** (valores de referência de 120 mA e 25 mA, respectivamente), refletindo
a hipótese de projeto de que o *uplink* Wi-Fi/MQTT concentra o maior custo de
comunicação. É essa diferença de custo por papel que dá às políticas baseadas em
energia margem para redistribuir o desgaste. A Seção 5.2 retoma exatamente essa
premissa ao confrontá-la com a medição em hardware.

### 5.1.1 Cenário A — carga plena (cluster homogêneo)

Com os cinco nós partindo de 100 % de carga e perfil de consumo idêntico, as três
políticas resultam em tempo de vida **estatisticamente equivalente**. A Figura 4
apresenta o *First Node Death* (FND) médio por política: os valores ficam contidos
em uma faixa de cerca de 2 % em torno da média (≈ 59,3, 58,3 e 57,9 min para
Round-Robin, energia e energia+*cooldown*), com barras de erro que se sobrepõem.
Como o estado inicial é o mesmo para todos os nós, não há desbalanceamento de
energia a corrigir, e a política de seleção de líder **não altera o instante de
quebra da redundância**.

A Figura 5 reforça essa leitura pelas curvas de depleção: em qualquer das três
políticas, os cinco nós descarregam de forma praticamente sobreposta e cruzam o
limiar de esgotamento quase juntos. O balanceamento da liderança também é
semelhante entre as políticas (σ entre 0,66 e 0,89 mandatos), sem vantagem
sistemática de nenhuma. Esse resultado é coerente com — e antecipa — o empate
observado em hardware (Seção 5.2): sob nós homogêneos, a rotação redistribui
*quem* lidera, mas não *quanto* a rede consome no agregado.

### 5.1.2 Cenário B — carga inicial escalonada

O valor das políticas baseadas em energia aparece quando os nós **partem
desbalanceados**. Neste cenário, os cinco nós têm a mesma capacidade mas iniciam
com cargas escalonadas (100 %, 85 %, 70 %, 55 % e 40 %), como no Cenário B de
bancada. A Figura 6 resume o resultado central do trabalho ao confrontar
**durabilidade** (FND) e **justiça** (σ do número de mandatos por nó) nas três
políticas.

Em durabilidade, a seleção por **energia residual estende o tempo de vida da rede
em cerca de 51 %** frente ao *Round-Robin* (≈ 26,0 min contra 17,2 min), e a
variante com *cooldown* o estende em cerca de 31 % (≈ 22,5 min). O mecanismo desse
ganho é evidenciado pelas curvas de depleção da Figura 7: sob *Round-Robin*, que
ignora a energia, todos os nós descarregam em paralelo e o nó que partiu com 40 %
atinge o esgotamento primeiro, antecipando o FND; sob a política de energia, o nó
mais carregado é levado a liderar (e a gastar) mais, de modo que as curvas
**convergem** e os nós chegam ao fim de vida praticamente juntos. Essa equalização
aparece também na dispersão da energia residual no FND, muito menor sob a política
de energia (0,35) do que sob *Round-Robin* (2,26): há menos carga "desperdiçada"
em nós sobreviventes no momento em que a redundância é perdida.

Esse ganho tem como contrapartida a **concentração da liderança**. A política de
energia pura é a mais desigual (σ ≈ 6,0): o nó mais fraco, protegido do papel caro,
quase nunca lidera. O *cooldown*, ao impedir a reeleição imediata do mesmo líder,
reduz essa concentração (σ ≈ 3,5) e distribui a coordenação de forma mais
homogênea, ao custo de parte do ganho de durabilidade (+31 % em vez de +51 %).
Configura-se, assim, o trade-off central: a energia pura maximiza o tempo de vida,
enquanto o *cooldown* troca uma fração desse tempo por um desgaste de coordenação
mais equilibrado entre os nós.

### 5.1.3 Síntese e ligação com o hardware

A simulação delimita com clareza o regime em que as políticas baseadas em energia
importam. Sob carga plena, as três estratégias empatam em tempo de vida; sob carga
inicial escalonada, a seleção por energia residual estende o FND de forma expressiva
(+51 %, ou +31 % com *cooldown*) ao equalizar a carga entre nós desiguais — emergindo
o trade-off entre maximizar a durabilidade (energia pura) e uniformizar o desgaste
de liderança (energia+*cooldown*). Em todos os casos, o ganho depende criticamente
da premissa de que **liderar custa mais que ser membro**: é o diferencial de custo
por papel que cria a energia a ser redistribuída. A Seção 5.2 testa essa premissa em
hardware e mostra que, na arquitetura adotada — com o rádio permanentemente ativo
para o ESP-NOW —, o custo de liderar e o de ser membro são praticamente iguais, o
que explica por que o ganho previsto pela simulação não se materializa na bancada.
Essa convergência entre modelo e medição, longe de invalidar a simulação, delimita
sua condição de validade e aponta o caminho de projeto para que a vantagem da
rotação se realize: diferenciar o consumo por papel — por exemplo, desligando o
rádio de longo alcance dos membros.

---

## Mapa de figuras (substituição)

| Figura nova | Arquivo | Substitui (versão antiga) |
|---|---|---|
| Fig. 4 — FND por política, Cenário A (homogêneo) | `fig_A_fnd_homogeneo.{png,pdf}` | Fig. 5 |
| Fig. 5 — Curvas de depleção, Cenário A | `fig_A_deplecao_homogeneo.{png,pdf}` | (nova) |
| Fig. 6 — Durabilidade × justiça, Cenário B (escalonado) | `fig_B_tradeoff_escalonado.{png,pdf}` | Fig. 4 |
| Fig. 7 — Curvas de depleção, Cenário B | `fig_B_deplecao_escalonado.{png,pdf}` | (nova) |

> Numeração das figuras é sugestão; ajuste à do seu documento. As antigas Fig. 6–10
> (uplink e plano de controle) não têm correspondente reproduzível e foram removidas
> nesta consolidação.
