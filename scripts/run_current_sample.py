#!/usr/bin/env python3
"""Collect a bounded, reproducible sample from the corrected benchmark."""

import argparse
import csv
import json
import os
import platform
import subprocess
import tempfile
from datetime import datetime, timezone
from pathlib import Path

from validate_benchmark_csv import HEADER, ORDERS, VARIANTS

SAMPLE_SIZES = (50_000, 100_000, 200_000)


def cpu_model() -> str:
    try:
        for line in Path("/proc/cpuinfo").read_text(encoding="utf-8").splitlines():
            if line.startswith("model name"):
                return line.partition(":")[2].strip()
    except OSError:
        pass
    return platform.processor() or "unknown"


def first_line(command: list[str]) -> str:
    try:
        return subprocess.check_output(command, text=True, stderr=subprocess.DEVNULL).splitlines()[0]
    except (OSError, subprocess.CalledProcessError, IndexError):
        return "unknown"


def cases():
    for variant in VARIANTS:
        for order in ORDERS:
            for size in SAMPLE_SIZES:
                # First-pivot ordered input is quadratic; one point demonstrates it.
                if variant == "first-pivot" and order != "random" and size > 50_000:
                    continue
                yield variant, order, size


def measure(binary: Path, seed: int, timeout: int, case: tuple[str, str, int]) -> list[str]:
    variant, order, size = case
    command = [str(binary), variant, order, str(size), "--seed", str(seed)]
    completed = subprocess.run(command, capture_output=True, text=True, check=True, timeout=timeout)
    fields = completed.stdout.strip().split()
    if len(fields) != len(HEADER) or fields[:3] != [variant, order, str(size)]:
        raise ValueError(f"Unexpected output for {variant} {order} {size}: {completed.stdout!r}")
    if any(not value.isdecimal() for value in fields[3:]):
        raise ValueError(f"Non-integer metric for {variant} {order} {size}")
    return fields


def write_csv(path: Path, rows: list[list[str]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile("w", newline="", encoding="utf-8", dir=path.parent,
                                     prefix=path.name + ".", delete=False) as stream:
        temporary = Path(stream.name)
        writer = csv.writer(stream, lineterminator="\n")
        writer.writerow(HEADER)
        writer.writerows(rows)
    os.replace(temporary, path)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, default=Path("./sort"))
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("--case-timeout", type=int, default=90)
    parser.add_argument("--compiler", default="gcc")
    parser.add_argument("--cflags", default="-O3")
    args = parser.parse_args()
    if args.seed < 0 or args.case_timeout < 1:
        parser.error("Seed must be nonnegative and case timeout must be positive")

    binary = args.binary.resolve()
    rows = []
    selected_cases = list(cases())
    started = datetime.now(timezone.utc)
    for index, case in enumerate(selected_cases, start=1):
        print(f"[{index}/{len(selected_cases)}] {case[0]} {case[1]} {case[2]}", flush=True)
        rows.append(measure(binary, args.seed, args.case_timeout, case))

    write_csv(args.output, rows)
    metadata = {
        "source": "corrected implementation, bounded sample",
        "started_utc": started.isoformat(),
        "finished_utc": datetime.now(timezone.utc).isoformat(),
        "git_commit": first_line(["git", "rev-parse", "HEAD"]),
        "host_os": platform.platform(),
        "cpu_model": cpu_model(),
        "compiler": first_line([args.compiler, "--version"]),
        "cflags": args.cflags,
        "seed": args.seed,
        "trials_per_case": 20,
        "time_unit": "microseconds",
        "random_generator": "(rand() % size) * 10 + 1; libc-dependent stream",
        "cases": len(rows),
        "selection": "50k, 100k, 200k for all variant/order pairs, except ordered first-pivot above 50k",
    }
    metadata_path = args.output.with_suffix(".metadata.json")
    metadata_path.write_text(json.dumps(metadata, indent=2) + "\n", encoding="utf-8")
    print(f"Saved {len(rows)} cases to {args.output} and {metadata_path}")


if __name__ == "__main__":
    main()
