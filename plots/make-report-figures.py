"""Recreate the three ns-3 plots used in the Word report."""
import argparse
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.ticker import FixedFormatter, FixedLocator, NullLocator


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, default=Path("report-figures"))
    args = parser.parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    data_dir = Path(__file__).resolve().parent.parent / "data"

    plt.rcParams.update({
        "font.family": "serif",
        "font.serif": ["Times New Roman", "Liberation Serif", "DejaVu Serif"],
        "font.size": 7.8,
        "axes.linewidth": 0.55,
    })
    for figure, (lower, ticks) in {
        6: (0.08, [0.1, 0.5, 1, 2, 5, 10]),
        7: (0.04, [0.05, 0.1, 0.5, 1, 2, 5, 10]),
        8: (0.4, [0.5, 1, 2, 5, 10]),
    }.items():
        table = data_dir / f"figure{figure}-goodput.dat"
        rows = [list(map(float, line.split())) for line in table.read_text().splitlines()
                if line.strip() and not line.lstrip().startswith("#")]
        rtts, *series = zip(*rows)
        fig, ax = plt.subplots(figsize=(3.12, 1.80))
        fig.subplots_adjust(left=0.15, right=0.98, bottom=0.19,
                            top=0.84 if figure == 7 else 0.78)
        labels = ("Hybla + SACK", "NewReno + SACK")
        if figure == 8:
            labels = ("Hybla, PER 0%", "Hybla, PER 1%", "Hybla, PER 5%")
        for values, label, marker, color in zip(
            series, labels, ("o", "s", "^"), ("#7c219c", "#00836e", "#287cc0")
        ):
            ax.plot(rtts, values, marker=marker, color=color,
                    linewidth=1.0, markersize=2.8, label=label)
        if figure in (6, 8):
            ax.axhline(10 / 6, color="#666666", linestyle="--",
                       linewidth=0.7, label="Mốc 10/6 Mbps")
        ax.set_yscale("log")
        ax.set_ylim(lower, 10)
        ax.set_xlim(-4, 635)
        ax.yaxis.set_major_locator(FixedLocator(ticks))
        ax.yaxis.set_major_formatter(FixedFormatter([str(t).replace(".", ",") for t in ticks]))
        ax.yaxis.set_minor_locator(NullLocator())
        ax.set_xticks([rtt for rtt in rtts if rtt != 25])
        ax.set_xlabel("RTT lan truyền mục tiêu (ms)", labelpad=2)
        ax.set_ylabel("Goodput (Mbps)", labelpad=2)
        ax.tick_params(axis="both", length=2, pad=2)
        ax.grid(True, which="major", linestyle=":", alpha=0.4, linewidth=0.55)
        handles, legend_labels = ax.get_legend_handles_labels()
        fig.legend(handles, legend_labels, loc="upper center",
                   bbox_to_anchor=(0.55, 1.0), frameon=False,
                   ncol=2, fontsize=6.8, columnspacing=0.9, handletextpad=0.35)
        fig.savefig(args.output_dir / f"report-figure{figure}.png", dpi=350, facecolor="white")
        plt.close(fig)


if __name__ == "__main__":
    main()
