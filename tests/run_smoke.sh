#!/bin/sh
set -eu

binary=$1

for variant in QC QM3 QPE QI1 QI5 QI10 QNR; do
    output=$("$binary" "$variant" OrdC 5)
    printf '%s\n' "$output" | awk -v variant="$variant" '
        NF != 6 || $1 != variant || $2 != "OrdC" || $3 != 5 ||
        $4 !~ /^[0-9]+$/ || $5 !~ /^[0-9]+$/ || $6 !~ /^[0-9]+$/ {
            exit 1
        }
    '
done

printed=$("$binary" QC OrdC 3 -p)
printf '%s\n' "$printed" | awk '
    NR == 1 && (NF != 6 || $1 != "QC" || $2 != "OrdC" || $3 != 3) { exit 1 }
    NR > 1 && $0 != "1 2 3 " { exit 1 }
    END { if (NR != 21) exit 1 }
'

if "$binary" QC OrdC invalid >/dev/null 2>&1; then
    echo "Invalid size was accepted" >&2
    exit 1
fi

echo "CLI smoke tests passed"
