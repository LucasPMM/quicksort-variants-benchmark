#!/bin/sh
set -eu

binary=$1

reject() {
    if output=$("$binary" "$@" 2>/dev/null); then
        echo "Invalid command was accepted: $*" >&2
        exit 1
    fi
    if [ -n "$output" ]; then
        echo "Invalid command wrote to stdout: $*" >&2
        exit 1
    fi
}

for variant in QC QM3 QPE QI1 QI5 QI10 QNR; do
    output=$("$binary" "$variant" OrdC 5)
    printf '%s\n' "$output" | awk -v variant="$variant" '
        NF != 6 || $1 != variant || $2 != "OrdC" || $3 != 5 ||
        $4 !~ /^[0-9]+$/ || $5 !~ /^[0-9]+$/ || $6 !~ /^[0-9]+$/ {
            exit 1
        }
    '
done

for order in ascending descending random; do
    output=$("$binary" middle-pivot "$order" 5 --seed 42)
    printf '%s\n' "$output" | awk -v order="$order" '
        NF != 6 || $1 != "middle-pivot" || $2 != order || $3 != 5 { exit 1 }
    '
done

help=$("$binary" --help)
case "$help" in
    *"median-of-three"*"microseconds"*) ;;
    *) echo "Help text is incomplete" >&2; exit 1 ;;
esac

printed=$("$binary" QC OrdC 3 -p)
printf '%s\n' "$printed" | awk '
    NR == 1 && (NF != 6 || $1 != "QC" || $2 != "OrdC" || $3 != 3) { exit 1 }
    NR > 1 && $0 != "1 2 3 " { exit 1 }
    END { if (NR != 21) exit 1 }
'

first=$("$binary" median-of-three random 10 --seed 42 --print | awk 'NR > 1')
second=$("$binary" median-of-three random 10 --print --seed 42 | awk 'NR > 1')
third=$("$binary" median-of-three random 10 --seed 43 --print | awk 'NR > 1')
if [ "$first" != "$second" ] || [ "$first" = "$third" ]; then
    echo "Seeded input is not reproducible" >&2
    exit 1
fi

reject
reject QC
reject QC OrdC
reject unknown OrdC 5
reject QC unknown 5
reject QC OrdC invalid
reject QC OrdC 0
reject QC OrdC -1
reject QC OrdC 500001
reject QC OrdC 5x
reject QC OrdC 999999999999999999999999
reject QC OrdC 5 --seed
reject QC OrdC 5 --seed -1
reject QC OrdC 5 --seed letters
reject QC OrdC 5 --seed 999999999999999999999999
reject QC OrdC 5 --seed 1 --seed 2
reject QC OrdC 5 -p -p
reject QC OrdC 5 --unknown

if [ -e /dev/full ] && "$binary" QC OrdC 3 >/dev/full 2>/dev/null; then
    echo "Output failure was not reported" >&2
    exit 1
fi

echo "CLI smoke tests passed"
