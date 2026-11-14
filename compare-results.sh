#!/usr/bin/env bash
set -euo pipefail
export LC_ALL=C

if [[ $# != 2 || ! ${1:-} =~ ^[678]$ ]]; then
    printf 'Usage: bash compare-results.sh {6|7|8} OUTPUT_DIR\n' >&2
    exit 2
fi

figure=$1
output=$2
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
name="figure$figure-goodput.dat"
reference="$root/data/$name"
if [[ "$output" != /* ]]; then output="$PWD/$output"; fi
measured="$output/data/$name"
for file in "$reference" "$measured"; do
    if [[ ! -f "$file" || ! -r "$file" ]]; then
        printf 'Missing or unreadable table: %s\n' "$file" >&2
        exit 1
    fi
done
cols=3
if [[ "$figure" == 8 ]]; then cols=4; fi

awk -v cols="$cols" -v figure="$figure" '
BEGIN { split("25 50 100 200 300 450 600", expected, " ") }
/^[[:space:]]*(#|$)/ { next }
{
    table = input
    row = ++rows[table]
    if (NF != cols || row > 7 || $1 !~ /^[0-9]+$/ || $1 + 0 != expected[row]) {
        printf "%s:%d: unexpected column count or RTT\n", FILENAME, FNR > "/dev/stderr"
        bad = 1
        exit 1
    }
    for (i = 2; i <= cols; i++) {
        if ($i !~ /^([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?$/) {
            printf "%s:%d: invalid goodput\n", FILENAME, FNR > "/dev/stderr"
            bad = 1
            exit 1
        }
        if (table == 1) {
            reference[row,i] = $i + 0
        } else {
            delta = $i - reference[row,i]
            if (delta < 0) delta = -delta
            if (delta > maximum) maximum = delta
        }
    }
}
END {
    if (bad) exit 1
    if (rows[1] != 7 || rows[2] != 7) {
        print "Both tables must contain seven RTT rows." > "/dev/stderr"
        exit 1
    }
    printf "Figure %s: %d rows compared, max |delta| = %.12g Mbps\n", figure, rows[2], maximum
}' input=1 "$reference" input=2 "$measured"
