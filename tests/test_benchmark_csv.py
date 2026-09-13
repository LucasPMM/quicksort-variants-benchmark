#!/usr/bin/env python3
"""Exercise the matrix runner and reject malformed benchmark datasets."""

import csv
import subprocess
import sys
import tempfile
import unittest
from itertools import product
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
from validate_benchmark_csv import HEADER, ORDERS, SIZES, VARIANTS, validate  # noqa: E402


class BenchmarkCsvTests(unittest.TestCase):
    def setUp(self) -> None:
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.path = Path(self.directory.name) / "matrix.csv"
        self.rows = [
            (variant, order, str(size), "1", "2", "3")
            for variant, order, size in product(VARIANTS, ORDERS, SIZES)
        ]

    def write_rows(self, rows, header=HEADER) -> None:
        with self.path.open("w", newline="", encoding="utf-8") as stream:
            writer = csv.writer(stream)
            writer.writerow(header)
            writer.writerows(rows)

    def test_complete_grid(self) -> None:
        self.write_rows(self.rows)
        validate(self.path)

    def test_missing_duplicate_and_extra_cases(self) -> None:
        for rows in (self.rows[:-1], self.rows + self.rows[:1], self.rows + [("other", "random", "50000", "1", "2", "3")]):
            with self.subTest(rows=len(rows)):
                self.write_rows(rows)
                with self.assertRaises(ValueError):
                    validate(self.path)

    def test_bad_schema_or_metrics(self) -> None:
        self.write_rows(self.rows, HEADER[:-1] + ("median_time_ms",))
        with self.assertRaises(ValueError):
            validate(self.path)
        for metric in ("-1", "abc"):
            with self.subTest(metric=metric):
                rows = self.rows.copy()
                rows[0] = rows[0][:-1] + (metric,)
                self.write_rows(rows)
                with self.assertRaises(ValueError):
                    validate(self.path)
        self.write_rows([self.rows[0] + ("extra",)] + self.rows[1:])
        with self.assertRaises(ValueError):
            validate(self.path)

    def test_runner_writes_valid_matrix_with_seed(self) -> None:
        stub = Path(self.directory.name) / "fake-sort.sh"
        stub.write_text(
            '#!/bin/sh\n[ "$4" = --seed ] && [ "$5" = 42 ] || exit 1\n'
            'printf "%s %s %s 1 2 3\\n" "$1" "$2" "$3"\n',
            encoding="utf-8",
        )
        stub.chmod(0o755)
        subprocess.run(
            ["sh", "scripts/run_benchmark_matrix.sh", str(stub), str(self.path), "42"],
            cwd=ROOT,
            check=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        validate(self.path)


if __name__ == "__main__":
    unittest.main()
