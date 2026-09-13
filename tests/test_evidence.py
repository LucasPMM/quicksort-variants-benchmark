#!/usr/bin/env python3
"""Validate the checked-in historical and corrected evidence before plotting."""

import csv
import json
import re
import subprocess
import sys
import tempfile
import unittest
from itertools import product
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
from run_current_sample import cases  # noqa: E402
from validate_benchmark_csv import HEADER, ORDERS, SIZES, VARIANTS  # noqa: E402

LEGACY_VARIANTS = ("QC", "QM3", "QPE", "QI1", "QI5", "QI10", "QNR")
LEGACY_ORDERS = ("OrdC", "OrdD", "Ale")


def load_rows(path: Path):
    with path.open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        if tuple(reader.fieldnames or ()) != HEADER:
            raise ValueError(f"Wrong header: {path}")
        rows = {}
        for raw in reader:
            if None in raw or any(value is None for value in raw.values()):
                raise ValueError(f"Wrong field count: {path}")
            row = {key: value.strip() for key, value in raw.items()}
            key = (row["variant"], row["input_order"], int(row["size"]))
            if key in rows:
                raise ValueError(f"Duplicate row: {key}")
            values = tuple(int(row[name]) for name in HEADER[3:])
            if any(value < 0 for value in values):
                raise ValueError(f"Negative metric: {key}")
            rows[key] = values
    return rows


class EvidenceTests(unittest.TestCase):
    def test_archive_contains_exact_historical_grid(self):
        expected = set(product(LEGACY_VARIANTS, LEGACY_ORDERS, SIZES))
        for filename in ("one-movement-per-swap.csv", "three-movements-per-swap.csv"):
            with self.subTest(filename=filename):
                path = ROOT / "historical-results" / filename
                self.assertEqual(set(load_rows(path)), expected)

    def test_archival_anchor_points(self):
        rows = load_rows(ROOT / "historical-results" / "three-movements-per-swap.csv")
        self.assertEqual(rows[("QPE", "OrdC", 500_000)][2], 84_256_470)
        self.assertEqual(rows[("QI10", "OrdC", 500_000)][2], 2_096)
        self.assertEqual(rows[("QI10", "Ale", 500_000)][2], 3_152_874)

    def test_corrected_sample_and_metadata(self):
        path = ROOT / "docs" / "results" / "corrected-sample-2026-09-13.csv"
        rows = load_rows(path)
        expected = set(cases())
        self.assertEqual(len(expected), 59)
        self.assertEqual(set(rows), expected)
        metadata = json.loads(path.with_suffix(".metadata.json").read_text(encoding="utf-8"))
        self.assertEqual(metadata["cases"], 59)
        self.assertEqual(metadata["trials_per_case"], 20)
        self.assertEqual(metadata["seed"], 42)
        self.assertEqual(metadata["time_unit"], "microseconds")
        self.assertEqual(set(VARIANTS), {case[0] for case in expected})
        self.assertEqual(set(ORDERS), {case[1] for case in expected})

    def test_sample_collector_and_readme_links(self):
        with tempfile.TemporaryDirectory() as directory:
            stub = Path(directory) / "fake-sort.sh"
            stub.write_text(
                '#!/bin/sh\n[ "$4" = --seed ] && [ "$5" = 42 ] || exit 1\n'
                'printf "%s %s %s 1 2 3\\n" "$1" "$2" "$3"\n',
                encoding="utf-8",
            )
            stub.chmod(0o755)
            output = Path(directory) / "sample.csv"
            subprocess.run(
                [sys.executable, "scripts/run_current_sample.py", "--binary", str(stub),
                 "--output", str(output), "--seed", "42", "--case-timeout", "1"],
                cwd=ROOT,
                check=True,
                capture_output=True,
                text=True,
            )
            self.assertEqual(set(load_rows(output)), set(cases()))
            metadata = json.loads(output.with_suffix(".metadata.json").read_text())
            self.assertEqual(metadata["cases"], 59)

        readme = (ROOT / "README.md").read_text(encoding="utf-8")
        for target in re.findall(r"\]\(([^)]+)\)", readme):
            if "://" not in target:
                self.assertTrue((ROOT / target).exists(), target)


if __name__ == "__main__":
    unittest.main()
