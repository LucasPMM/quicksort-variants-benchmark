#!/usr/bin/env python3
"""Render English-labeled figures from archived and corrected benchmark CSVs."""

import argparse
import csv
import os
from itertools import product
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
os.environ.setdefault("MPLCONFIGDIR", str(ROOT / "build" / "matplotlib"))

import matplotlib.pyplot as plt  # noqa: E402
from matplotlib.ticker import FuncFormatter  # noqa: E402

from run_current_sample import cases  # noqa: E402
from validate_benchmark_csv import HEADER, ORDERS, SIZES, VARIANTS  # noqa: E402

LEGACY_VARIANTS = {
    "middle-pivot": "QC",
    "median-of-three": "QM3",
    "first-pivot": "QPE",
    "hybrid-1": "QI1",
    "hybrid-5": "QI5",
    "hybrid-10": "QI10",
    "iterative": "QNR",
}
LEGACY_ORDERS = {"ascending": "OrdC", "descending": "OrdD", "random": "Ale"}
LABELS = {
    "middle-pivot": "Middle pivot (QC)",
    "median-of-three": "Median of three (QM3)",
    "first-pivot": "First pivot (QPE)",
    "hybrid-1": "Hybrid 1% (QI1)",
    "hybrid-5": "Hybrid 5% (QI5)",
    "hybrid-10": "Hybrid 10% (QI10)",
    "iterative": "Iterative (QNR)",
}
COLORS = dict(zip(VARIANTS, plt.get_cmap("tab10").colors[: len(VARIANTS)]))


def load_table(path: Path, expected: set[tuple[str, str, int]]) -> dict:
    """Verify every source row before passing its exact values to a plot."""
    rows = {}
    with path.open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        if tuple(reader.fieldnames or ()) != HEADER:
            raise ValueError(f"Unexpected CSV header in {path}")
        for line_number, raw in enumerate(reader, start=2):
            if None in raw or any(value is None for value in raw.values()):
                raise ValueError(f"Wrong field count at {path}:{line_number}")
            row = {key: value.strip() for key, value in raw.items()}
            try:
                key = (row["variant"], row["input_order"], int(row["size"]))
                metrics = {column: int(row[column]) for column in HEADER[3:]}
            except ValueError as error:
                raise ValueError(f"Non-integer value at {path}:{line_number}") from error
            if key not in expected or key in rows or any(value < 0 for value in metrics.values()):
                raise ValueError(f"Unexpected, repeated, or negative row at {path}:{line_number}")
            rows[key] = metrics
    if rows.keys() != expected:
        raise ValueError(f"Missing {len(expected - rows.keys())} expected rows in {path}")
    return rows


def historical_key(variant: str, order: str, size: int) -> tuple[str, str, int]:
    return LEGACY_VARIANTS[variant], LEGACY_ORDERS[order], size


def style_axis(axis, ylabel: str) -> None:
    axis.set_xlabel("Input size (elements)")
    axis.set_ylabel(ylabel)
    axis.xaxis.set_major_formatter(FuncFormatter(lambda value, _: f"{value / 1000:.0f}k"))
    axis.grid(alpha=0.24, linewidth=0.8)
    axis.spines[["top", "right"]].set_visible(False)


def draw_series(axis, rows: dict, order: str, metric: str, historical: bool,
                variants=VARIANTS, sizes=SIZES) -> None:
    for variant in variants:
        points = []
        for size in sizes:
            key = historical_key(variant, order, size) if historical else (variant, order, size)
            if key in rows:
                points.append((size, rows[key][metric]))
        if points:
            axis.plot([point[0] for point in points], [point[1] for point in points],
                      marker="o", markersize=3.2, linewidth=1.9,
                      color=COLORS[variant], label=LABELS[variant])


def save(fig, path: Path) -> None:
    fig.savefig(path, dpi=180, bbox_inches="tight", facecolor="white")
    plt.close(fig)
    print(f"Rendered {path}")


def plot_ordered(rows: dict, output: Path, zoom: bool) -> None:
    fig, axes = plt.subplots(1, 2, figsize=(12, 5), sharey=True)
    variants = tuple(variant for variant in VARIANTS if variant != "first-pivot") if zoom else VARIANTS
    for axis, order in zip(axes, ("ascending", "descending")):
        draw_series(axis, rows, order, "median_time_us", True, variants=variants)
        axis.set_title(order.capitalize())
        style_axis(axis, "Median sort time (µs)")
    fig.suptitle("2019 archive: ordered input" + (" (excluding first pivot)" if zoom else ""),
                 fontsize=14)
    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc="lower center", ncol=3, frameon=False, fontsize=9)
    fig.tight_layout(rect=(0, 0.11, 1, 0.94))
    save(fig, output)


def plot_random(rows: dict, output: Path, metric: str) -> None:
    fig, axis = plt.subplots(figsize=(9.5, 5.5))
    draw_series(axis, rows, "random", metric, True)
    axis.set_title("2019 archive: random input", fontsize=14)
    style_axis(axis, "Median sort time (µs)" if metric == "median_time_us" else "Mean movements")
    axis.legend(loc="upper left", ncol=2, frameon=False, fontsize=8.5)
    fig.tight_layout()
    save(fig, output)


def plot_comparison(historical: dict, current: dict, output: Path) -> None:
    fig, axes = plt.subplots(1, 2, figsize=(12, 5.2), sharex=True, sharey=True)
    for axis, rows, archived, title in zip(
        axes, (historical, current), (True, False),
        ("2019 archive", "2026 corrected sample"),
    ):
        draw_series(axis, rows, "random", "median_time_us", archived,
                    sizes=(50_000, 100_000, 200_000))
        axis.set_title(title)
        axis.set_yscale("log")
        style_axis(axis, "Median sort time (µs, log scale)")
    fig.suptitle("Random input: historical and current results", fontsize=14)
    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc="lower center", ncol=3, frameon=False, fontsize=9)
    fig.tight_layout(rect=(0, 0.11, 1, 0.94))
    save(fig, output)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--historical", type=Path,
                        default=ROOT / "historical-results" / "three-movements-per-swap.csv")
    parser.add_argument("--current", type=Path,
                        default=ROOT / "docs" / "results" / "corrected-sample-2026-09-13.csv")
    parser.add_argument("--output-dir", type=Path, default=ROOT / "docs" / "figures")
    args = parser.parse_args()
    legacy_expected = set(product(LEGACY_VARIANTS.values(), LEGACY_ORDERS.values(), SIZES))
    historical = load_table(args.historical, legacy_expected)
    current = load_table(args.current, set(cases()))
    args.output_dir.mkdir(parents=True, exist_ok=True)
    plot_ordered(historical, args.output_dir / "historical-ordered-time.png", zoom=False)
    plot_ordered(historical, args.output_dir / "historical-ordered-time-zoom.png", zoom=True)
    plot_random(historical, args.output_dir / "historical-random-time.png", "median_time_us")
    plot_random(historical, args.output_dir / "historical-random-movements.png", "mean_movements")
    plot_comparison(historical, current, args.output_dir / "random-time-comparison.png")


if __name__ == "__main__":
    main()
