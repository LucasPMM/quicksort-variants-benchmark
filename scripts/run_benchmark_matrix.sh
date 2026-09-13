#!/bin/sh
set -eu

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <sort_binary> <output_csv> <seed>" >&2
    exit 2
fi

binary=$1
output=$2
seed=$3
script_directory=$(CDPATH= cd "$(dirname "$0")" && pwd)
mkdir -p "$(dirname "$output")"
temporary=$(mktemp "${output}.tmp.XXXXXX")
trap 'rm -f "$temporary"' EXIT HUP INT TERM

printf '%s\n' 'variant,input_order,size,mean_comparisons,mean_movements,median_time_us' > "$temporary"

# Every variant sees the same seeded random stream for a given input size.
for variant in middle-pivot median-of-three first-pivot hybrid-1 hybrid-5 hybrid-10 iterative; do
    for order in ascending descending random; do
        for size in 50000 100000 150000 200000 250000 300000 350000 400000 450000 500000; do
            printf 'Running %s %s %s\n' "$variant" "$order" "$size" >&2
            result=$("$binary" "$variant" "$order" "$size" --seed "$seed")
            printf '%s\n' "$result" | awk -v variant="$variant" -v order="$order" -v size="$size" '
                NF != 6 || $1 != variant || $2 != order || $3 != size ||
                $4 !~ /^[0-9]+$/ || $5 !~ /^[0-9]+$/ || $6 !~ /^[0-9]+$/ {
                    exit 1
                }
                { printf "%s,%s,%s,%s,%s,%s\n", $1, $2, $3, $4, $5, $6 }
                END { if (NR != 1) exit 1 }
            ' >> "$temporary"
        done
    done
done

python3 "$script_directory/validate_benchmark_csv.py" "$temporary"
mv "$temporary" "$output"
trap - EXIT HUP INT TERM
printf 'Saved validated results to %s\n' "$output"
