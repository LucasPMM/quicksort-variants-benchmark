#!/usr/bin/env python3
"""Validate the complete, microsecond-based quicksort benchmark matrix."""

import argparse
import csv
from itertools import product
from pathlib import Path

VARIANTS = (
    "middle-pivot",
    "median-of-three",
    "first-pivot",
    "hybrid-1",
    "hybrid-5",
    "hybrid-10",
    "iterative",
)
ORDERS = ("ascending", "descending", "random")
SIZES = tuple(range(50_000, 500_001, 50_000))
HEADER = (
    "variant",
    "input_order",
    "size",
    "mean_comparisons",
    "mean_movements",
    "median_time_us",
)


def validate(path: Path) -> None:
    """Raise ValueError if the CSV is not exactly the documented 7 × 3 × 10 grid."""
    expected = set(product(VARIANTS, ORDERS, SIZES))
    seen = set()
    with path.open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        if tuple(reader.fieldnames or ()) != HEADER:
            raise ValueError("Expected the canonical CSV header ending in median_time_us")
        for line_number, row in enumerate(reader, start=2):
            if None in row or any(value is None for value in row.values()):
                raise ValueError(f"Line {line_number} has the wrong number of fields")
            try:
                size = int(row["size"])
                metrics = [int(row[name]) for name in HEADER[3:]]
            except ValueError as error:
                raise ValueError(f"Line {line_number} has a non-integer field") from error
            if any(value < 0 for value in metrics):
                raise ValueError(f"Line {line_number} has a negative metric")
            key = (row["variant"], row["input_order"], size)
            if key not in expected:
                raise ValueError(f"Line {line_number} is outside the benchmark matrix")
            if key in seen:
                raise ValueError(f"Line {line_number} duplicates a benchmark case")
            seen.add(key)
    if seen != expected:
        raise ValueError(f"Expected {len(expected)} cases, found {len(seen)}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv_file", type=Path)
    args = parser.parse_args()
    try:
        validate(args.csv_file)
    except (OSError, ValueError) as error:
        parser.exit(1, f"Invalid benchmark CSV: {error}\n")
    print(f"Validated {len(VARIANTS) * len(ORDERS) * len(SIZES)} benchmark rows")


if __name__ == "__main__":
    main()
