# Legacy and corrected behavior comparison

This comparison uses the [2019 report](../historical-results/original-report-2019.pdf),
the archived [three-movements-per-swap CSV](../historical-results/three-movements-per-swap.csv),
the [corrected sample](results/corrected-sample-2026-09-13.csv), and an independent
[same-host replication](results/corrected-replication-2026-09-13.csv). The archived CSV has
210 cases; each corrected run has 59 overlapping cases. All corrected cases use 20 trials,
seed `42`, GCC 13.3.0 with `-O3`, the same AMD Ryzen 5 3600 host and C library, and
microseconds for time. Ordered first-pivot cases above 50,000 elements were excluded from
the corrected sample because their quadratic cost makes the full matrix much longer.

The original report describes GCC 7.4.0, Ubuntu 18.04.2, an Intel Core i5-7200U, and a
different, unrecorded random seed. Its timing-median implementation accessed outside its
20-element array. The archived times are evidence of what the 2019 run recorded, not a
controlled performance baseline for the repaired implementation. The original time header
said milliseconds, but the report and source calculation specify **microseconds (µs)**.

## Expected behavior and compatibility

| Reported contract | Corrected implementation | Assessment |
| --- | --- | --- |
| Seven variants: middle (`QC`), median of three (`QM3`), first (`QPE`), 1%/5%/10% insertion hybrids (`QI1`/`QI5`/`QI10`), and iterative middle-pivot (`QNR`). | All seven strategies remain available under their original codes and English names. | Preserved; the median-of-three and iterative implementations now perform their stated operations correctly. |
| Ascending, descending, and generated random inputs; study sizes 50,000–500,000 in 50,000-element steps. | `OrdC`/`OrdD`/`Ale` remain aliases; the generator and study grid are unchanged. The CLI also accepts 1–500,000 for examples and tests. | Preserved with a documented CLI extension. |
| Twenty arrays per run; mean comparisons, mean movements, and median sort time in the six-field result line. | Same field order and trial count. Only sorting is timed; an optional seed makes a run repeatable on the same C library. | Preserved, with safe median arithmetic and a monotonic clock. |
| Three movements per partition swap in the main analysis; an alternative one-movement convention was also archived. | Three movements per swap, including self-swaps; the alternative CSV remains separate. | Preserved. |
| Optional `-p` prints the original arrays after the result. | `-p` remains accepted; `--print` is an alias. | Preserved. |

The report's library story assumes unique book identifiers, but the actual 2019 generator
used `(rand() % n) * 10 + 1`, which can repeat values. The corrected program deliberately
keeps that distribution for continuity. Uniqueness should be treated as narrative context,
not as a property of either measured dataset. The report also recommends increasing the
process stack for large inputs. The corrected recursive sorter works on the smaller
partition recursively and handles the larger partition in a loop, so that old workaround
is no longer part of the build/run instructions.

## Operation counts: 2019 archive versus corrected code

Ordered inputs are deterministic, so their comparison counts provide a more useful
compatibility check than elapsed time. The table shows all seven variants at 50,000
elements; each cell is **archive → corrected sample**. The same patterns hold at 100,000
and 200,000 elements where both datasets have a case.

| Variant | Ascending mean comparisons | Descending mean comparisons |
| --- | ---: | ---: |
| `QC` | 750,015 → 750,015 | 750,028 → 750,028 |
| `QM3` | 750,015 → 750,015 | 750,028 → 750,028 |
| `QPE` | 1,250,074,998 → 1,250,074,998 | 1,250,099,996 → 1,250,099,996 |
| `QI1` | 399,879 → 399,752 | 399,885 → 399,758 |
| `QI5` | 299,973 → 299,942 | 299,977 → 299,946 |
| `QI10` | 249,988 → 249,973 | 249,991 → 249,976 |
| `QNR` | 750,000 → 750,015 | 750,000 → 750,028 |

`QC`, `QM3`, and `QPE` match all available ordered comparison counts exactly: 6/6 for
each of `QC` and `QM3`, and 2/2 for `QPE`. On monotonic input, the median of first,
middle, and last is the middle value, so this agreement alone does not test the repaired
random-input median-of-three pivot; direct pivot tests do. `QPE` still exhibits its
quadratic ordered-input behavior. At 50,000 elements, its ascending and descending
movement counts also match the archive exactly at 149,997.

The hybrid comparison deltas are consistently −127 (`QI1`), −31 (`QI5`), and −15
(`QI10`) at all three shared ordered sizes. The old insertion loop checked `j >= 0`
instead of the current partition's left bound. Even on already ordered data, that could
count one comparison against the preceding partition for each noninitial insertion
subrange. The corrected loop respects the subrange boundary; these count changes are
expected rather than a change in variant identity. The old `QNR` omitted the final array
element. Its missing comparisons explain the small increase after the iterative traversal
was corrected. In the corrected sample, `QNR` and recursive middle-pivot `QC` have
identical comparison **and** movement counts in all nine shared input/order/size cases.
Some archived ordered movement means differ from corrected counts by one: the old program
accumulated floating-point per-trial means and then truncated them, while the corrected
program averages integer totals. This is consistent with old pure-quicksort movement
values that are not multiples of three; the archival rows are not edited.

Random inputs cannot provide exact cross-era counter agreement because the 2019 seed is
unknown, the C-library stream may differ, and several variants were repaired. At 200,000
random elements, for example, archived `QM3` records 4,734,081 comparisons and the
corrected sample records 4,319,563. The old recursive `QM3` effectively selected the
middle pivot for normal ranges; the corrected implementation selects the median of three.
The combined effect of that fix and the different inputs cannot be separated from the
archived CSV alone. The corrected `QC` and `QNR` both record 4,788,211 comparisons and
2,664,725 movements for the same seeded random inputs, as expected from their shared
pivot and partition policy.

## Time trends and an independent replication

The [original ascending-time plot](../historical-results/plots/three-movements-per-swap/ascending-time.png)
and the [English re-rendering](figures/historical-ordered-time.png) have the same archived
curve and endpoint: `QPE` reaches 84,256,470 µs at 500,000 elements. Likewise, the
[original random-time plot](../historical-results/plots/three-movements-per-swap/random-time.png)
and [English re-rendering](figures/historical-random-time.png) both end at 3,152,874 µs
for `QI10`. The [cross-era figure](figures/random-time-comparison.png) displays the 2019
and corrected samples separately on the same microsecond scale.

The broad qualitative results survive the repairs: first-pivot quicksort degenerates on
ordered arrays; the 10% insertion hybrid has the fewest comparisons and shortest recorded
ordered times at the three corrected sample sizes; and larger insertion cutoffs increase
random-input work, with `QI10` slowest on random input. A specific ranking is less stable:
the corrected `QI1` is the shortest recorded random-input case at 50,000 elements, whereas
the report broadly calls the insertion hybrids the slowest on random input. The report's
claim that `QC`, `QM3`, and `QNR` have identical movements in *all* historical cases is
also contradicted by its random-input CSV rows. Its description of ordered `QPE`
comparison growth as exponential should read quadratic.

The independent corrected replication completed all 59 cases with the same host, compiler,
flags, seed, trial count, and C library. **All 59 comparison means and all 59 movement
means match the first corrected sample exactly.** The ratio of replicated to first-run
median time has median 0.998, but individual ratios range from 0.652 to 2.039. This
demonstrates why time is not asserted as an exact regression value, even on one machine.
For instance, the first run measured descending `QI10` at 50,000 elements as 437 µs;
the replication measured 285 µs. No old-versus-new speedup factor is claimed.

The first sample's metadata records the commit at measurement time, `88948f0`. GitHub's
rebase merge rewrote that commit; the published equivalent is `63a1b2b`, with the same
Git tree. The replication records the merged `master` commit `bfd44bd`. This makes both
measurements traceable without altering their observed values. The unsafe 2019 binary was
not rerun; a controlled same-hardware legacy comparison would require a separately labeled
replay with its undefined timing path repaired and identical input fixtures.
