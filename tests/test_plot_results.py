#!/usr/bin/env python3
"""Ensure every plotted point is the exact value in its source CSV."""

import os
import sys
import unittest
from itertools import product
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
os.environ.setdefault("MPLCONFIGDIR", str(ROOT / "build" / "matplotlib"))

import matplotlib.pyplot as plt  # noqa: E402
from plot_results import (  # noqa: E402
    LEGACY_ORDERS,
    LEGACY_VARIANTS,
    draw_series,
    historical_key,
    load_table,
)
from run_current_sample import cases  # noqa: E402
from validate_benchmark_csv import SIZES, VARIANTS  # noqa: E402


class FigureDataTests(unittest.TestCase):
    def test_all_archival_time_points(self):
        expected = set(product(LEGACY_VARIANTS.values(), LEGACY_ORDERS.values(), SIZES))
        rows = load_table(ROOT / "historical-results" / "three-movements-per-swap.csv", expected)
        for order in ("ascending", "descending", "random"):
            with self.subTest(order=order):
                fig, axis = plt.subplots()
                draw_series(axis, rows, order, "median_time_us", True)
                self.assertEqual(len(axis.lines), len(VARIANTS))
                for line, variant in zip(axis.lines, VARIANTS):
                    self.assertEqual(list(line.get_xdata()), list(SIZES))
                    self.assertEqual(
                        list(line.get_ydata()),
                        [rows[historical_key(variant, order, size)]["median_time_us"]
                         for size in SIZES],
                    )
                plt.close(fig)

    def test_corrected_random_time_points(self):
        rows = load_table(
            ROOT / "docs" / "results" / "corrected-sample-2026-09-13.csv", set(cases())
        )
        fig, axis = plt.subplots()
        sizes = (50_000, 100_000, 200_000)
        draw_series(axis, rows, "random", "median_time_us", False, sizes=sizes)
        self.assertEqual(len(axis.lines), len(VARIANTS))
        for line, variant in zip(axis.lines, VARIANTS):
            self.assertEqual(list(line.get_xdata()), list(sizes))
            self.assertEqual(
                list(line.get_ydata()),
                [rows[(variant, "random", size)]["median_time_us"] for size in sizes],
            )
        plt.close(fig)


if __name__ == "__main__":
    unittest.main()
