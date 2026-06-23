#!/usr/bin/env python3
"""Gera as figuras curadas da Seção 5.1 (Simulação) e despeja as métricas reais.

Reproduz, a partir do simulador fiel ao firmware (analysis/), o conjunto enxuto de
figuras usado na reescrita da Seção 5.1, organizado nos dois cenários dos ensaios
de laboratório (5 nós):

  Cenário A — homogêneo (todos os nós começam com 100% de carga):
    * FND por política              -> tempo de vida da rede (empate esperado)
    * curvas de depleção por nó     -> intuição do desgaste uniforme

  Cenário B — carga inicial escalonada (100/85/70/55/40%, mesma capacidade):
    * FND e σ da liderança por política -> durabilidade vs justiça, com o
      baseline Round-Robin que evidencia o ganho da seleção por energia
    * curvas de depleção por nó     -> mecanismo: a política de energia equaliza
      a carga e os nós morrem juntos; o Round-Robin deixa o nó mais fraco morrer
      cedo

Tudo deriva do mesmo motor de simulação (passo de 100 ms, parâmetros
sincronizados com o firmware). Nada é digitado à mão: rode e os PNG/PDF saem nesta
pasta, e as métricas vão para `metricas.md`.

Uso (da raiz do repo):
    python resultados_do_desespero/gerar_figuras.py
"""
import os
import sys
from dataclasses import replace

# Permite rodar tanto como módulo quanto como script solto.
_HERE = os.path.dirname(os.path.abspath(__file__))
_ROOT = os.path.dirname(_HERE)
if _ROOT not in sys.path:
    sys.path.insert(0, _ROOT)

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd
from analysis.simulator import sim
from analysis.simulator.energy import ABSTRACT, calibrated
from analysis import metrics, calibration, params

OUTDIR = _HERE
POLICIES = ("round_robin", "energy", "energy_cooldown")
SEEDS = (1, 2, 3, 4, 5)
CLUSTER_SIZE = 5                       # mesmos 5 nós dos ensaios de laboratório
# Cenário B: carga inicial escalonada, idêntica ao firmware
# (main/src/Application/reset_service.cpp -> STAGGER_LEVELS, por ordem de MAC).
LEVELS_PCT_B = (100, 85, 70, 55, 40)
CAP_B = 10.0                           # capacidade comum (unidades do modelo)
LABEL = {"round_robin": "Round-Robin", "energy": "Energia",
         "energy_cooldown": "Energia+Cooldown"}
COLOR = "#3b6ea5"


# ----------------------------------------------------------------------------- #
# Geração dos cenários                                                          #
# ----------------------------------------------------------------------------- #
def _homog_df():
    """Cenário A: 5 nós idênticos (perfil abstrato, todos a 100%)."""
    frames = [sim.run_frame(CLUSTER_SIZE, pol, ABSTRACT, s)
              for pol in POLICIES for s in SEEDS]
    return pd.concat(frames, ignore_index=True)


def _scenarioB_df():
    """Cenário B: 5 nós de MESMA capacidade, cargas iniciais escalonadas
    (100/85/70/55/40%). A energia absoluta inicial é o que o algoritmo enxerga e
    o que determina o FND."""
    base = calibrated(calibration.LEADER_MA, calibration.MEMBER_MA,
                      calibration.IDLE_MA, capacity_mah=CAP_B)
    profiles = [replace(base, initial=CAP_B * p / 100.0) for p in LEVELS_PCT_B]
    frames = [sim.run_frame(CLUSTER_SIZE, pol, base, s, profiles=profiles)
              for pol in POLICIES for s in SEEDS]
    return pd.concat(frames, ignore_index=True)


# ----------------------------------------------------------------------------- #
# Plotagem (local, para títulos/legendas corretos por cenário)                  #
# ----------------------------------------------------------------------------- #
def _save(fig, name):
    png = os.path.join(OUTDIR, name + ".png")
    fig.savefig(png, dpi=150, bbox_inches="tight")
    fig.savefig(os.path.join(OUTDIR, name + ".pdf"), bbox_inches="tight")
    plt.close(fig)
    return png


def _fnd_min(df):
    d = metrics.fnd_by_policy(df).set_index("policy").reindex(POLICIES)
    return d["fnd_ms_mean"] / 60000.0, d["fnd_ms_std"] / 60000.0


def _fig_fnd_homog(df, name):
    mean, std = _fnd_min(df)
    fig, ax = plt.subplots(figsize=(5, 3.2))
    ax.bar([LABEL[p] for p in POLICIES], [mean[p] for p in POLICIES],
           yerr=[std[p] for p in POLICIES], capsize=4, color=COLOR)
    ax.set_ylabel("FND (min)")
    ax.set_title("Cenário A (homogêneo) — tempo de vida por política")
    return _save(fig, name)


def _fig_tradeoff_B(df, name):
    """Cenário B: durabilidade (FND) e justiça (σ da liderança), 3 políticas.
    Anota o ganho de FND de cada política sobre o baseline Round-Robin."""
    mean, std = _fnd_min(df)
    sg = metrics.leadership_std(df).set_index("policy").reindex(POLICIES)["std"]
    rr = mean["round_robin"]
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(9, 3.6))
    xs = list(range(len(POLICIES)))
    bars = ax1.bar(xs, [mean[p] for p in POLICIES],
                   yerr=[std[p] for p in POLICIES], capsize=4, color=COLOR)
    for x, p, b in zip(xs, POLICIES, bars):
        if p != "round_robin":
            ax1.annotate(f"{100*(mean[p]-rr)/rr:+.0f}%",
                         (x, b.get_height() + std[p]),
                         textcoords="offset points", xytext=(0, 6),
                         ha="center", fontsize=9, fontweight="bold")
    ax1.set_ylim(top=max(mean) * 1.18)
    ax1.set_ylabel("FND (min)")
    ax1.set_title("Durabilidade (maior = melhor)")
    ax2.bar(xs, [sg[p] for p in POLICIES], color=COLOR)
    ax2.set_ylabel("Desvio-padrão da liderança (σ)")
    ax2.set_title("Justiça (menor = melhor)")
    for ax in (ax1, ax2):
        ax.set_xticks(xs)
        ax.set_xticklabels([LABEL[p] for p in POLICIES], rotation=12, ha="right")
    fig.suptitle("Cenário B (carga inicial escalonada 40–100%): durabilidade × justiça")
    fig.tight_layout(rect=[0, 0, 1, 0.93])
    return _save(fig, name)


def _fig_depletion(df, name, title, capacity=None):
    """Curvas de depleção por nó (uma execução). Se `capacity` é dado, normaliza
    a energia residual absoluta por essa capacidade comum (Cenário B, para as
    curvas partirem dos níveis escalonados); senão usa o residual_pct do contrato
    (Cenário A)."""
    s = df[df["event"] == "sample"].copy()
    if capacity is not None:
        s["pct"] = 100.0 * s["residual"] / capacity
    else:
        s["pct"] = s["residual_pct"]
    pols = [p for p in POLICIES if p in set(s["policy"])]
    fig, axes = plt.subplots(1, len(pols), figsize=(4 * len(pols), 3.2), sharey=True)
    if len(pols) == 1:
        axes = [axes]
    for ax, pol in zip(axes, pols):
        sub = s[s["policy"] == pol]
        one = sub[sub["run_id"] == sorted(set(sub["run_id"]))[0]]
        for node, g in one.groupby("node_id"):
            g = g.sort_values("t_ms")
            ax.plot(g["t_ms"] / 60000.0, g["pct"], label=node)
        ax.set_title(LABEL[pol]); ax.set_xlabel("Tempo (min)")
    axes[0].set_ylabel("Energia residual (%)")
    axes[-1].legend(title="Nó", fontsize=8)
    fig.suptitle(title)
    fig.tight_layout(rect=[0, 0, 1, 0.93])
    return _save(fig, name)


# ----------------------------------------------------------------------------- #
# Métricas em markdown                                                          #
# ----------------------------------------------------------------------------- #
def _fnd_rows(df):
    mean, std = _fnd_min(df)
    rr = mean["round_robin"]
    rows = []
    for p in POLICIES:
        gain = "—" if p == "round_robin" else f"{100*(mean[p]-rr)/rr:+.1f}%"
        rows.append(f"| {LABEL[p]} | {mean[p]:.2f} | {std[p]:.2f} | {gain} |")
    return "\n".join(rows)


def _simple_rows(series):
    return "\n".join(f"| {LABEL[p]} | {series[p]:.3f} |" for p in POLICIES)


def main():
    homog = _homog_df()
    scenB = _scenarioB_df()

    paths = [
        _fig_fnd_homog(homog, "fig_A_fnd_homogeneo"),
        _fig_depletion(homog, "fig_A_deplecao_homogeneo",
                       "Cenário A (homogêneo) — curvas de depleção por nó"),
        _fig_tradeoff_B(scenB, "fig_B_tradeoff_escalonado"),
        _fig_depletion(scenB, "fig_B_deplecao_escalonado",
                       "Cenário B (escalonado 40–100%) — curvas de depleção por nó",
                       capacity=CAP_B),
    ]

    sgA = metrics.leadership_std(homog).set_index("policy").reindex(POLICIES)["std"]
    spA = metrics.residual_spread(homog).set_index("policy").reindex(POLICIES)["spread"]
    sgB = metrics.leadership_std(scenB).set_index("policy").reindex(POLICIES)["std"]
    spB = metrics.residual_spread(scenB).set_index("policy").reindex(POLICIES)["spread"]

    with open(os.path.join(OUTDIR, "metricas.md"), "w", encoding="utf-8") as f:
        f.write("# Métricas reais do simulador (geradas por gerar_figuras.py)\n\n")
        f.write(f"Correntes assumidas (placeholder de calibração): "
                f"LÍDER={calibration.LEADER_MA} mA, MEMBRO={calibration.MEMBER_MA} mA, "
                f"IDLE={calibration.IDLE_MA} mA. {CLUSTER_SIZE} nós, seeds={list(SEEDS)}, "
                f"mandato={params.TERM_DURATION_MS//1000} s, "
                f"cooldown={params.COOLDOWN_MS//1000} s.\n\n")

        f.write("## Cenário A — homogêneo (5 nós idênticos, 100%)\n\n")
        f.write("| Política | FND médio (min) | Desvio (min) | vs Round-Robin |\n")
        f.write("|---|---|---|---|\n" + _fnd_rows(homog) + "\n\n")
        f.write("σ da liderança (menor = mais justo):\n\n")
        f.write("| Política | σ |\n|---|---|\n" + _simple_rows(sgA) + "\n\n")
        f.write("Dispersão da energia residual no FND (menor = menos desperdício):\n\n")
        f.write("| Política | spread |\n|---|---|\n" + _simple_rows(spA) + "\n\n")

        f.write(f"## Cenário B — carga inicial escalonada {'/'.join(map(str, LEVELS_PCT_B))}% "
                f"(mesma capacidade)\n\n")
        f.write("| Política | FND médio (min) | Desvio (min) | vs Round-Robin |\n")
        f.write("|---|---|---|---|\n" + _fnd_rows(scenB) + "\n\n")
        f.write("σ da liderança (menor = mais justo):\n\n")
        f.write("| Política | σ |\n|---|---|\n" + _simple_rows(sgB) + "\n\n")
        f.write("Dispersão da energia residual no FND (menor = menos desperdício):\n\n")
        f.write("| Política | spread |\n|---|---|\n" + _simple_rows(spB) + "\n")

    print("Figuras geradas em:", OUTDIR)
    for p in paths:
        print("  -", os.path.basename(p))
    print("Métricas em: metricas.md")


if __name__ == "__main__":
    main()
