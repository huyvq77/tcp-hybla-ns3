#!/usr/bin/env bash
set -euo pipefail
export LC_ALL=C

if (( $# != 1 )); then
    printf 'Usage: bash run-figures.sh /path/to/ns-3.49\n' >&2
    exit 2
fi

package_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
ns3_dir=$(cd -- "$1" && pwd)
if [[ ! -x "$ns3_dir/ns3" || ! -f "$ns3_dir/scratch/hybla-experiment.cc" ]]; then
    printf 'Install the project sources in ns-3.49 first (see README.md).\n' >&2
    exit 2
fi

read -r -a figures <<< "${FIGURES-6 7 8}"
if (( ${#figures[@]} == 0 )); then
    printf 'Select one or more figures (6, 7, 8).\n' >&2
    exit 2
fi
selected=' '
for figure in "${figures[@]}"; do
    if [[ ! "$figure" =~ ^[678]$ || "$selected" == *" $figure "* ]]; then
        printf 'Invalid or repeated figure: %s\n' "$figure" >&2
        exit 2
    fi
    selected+="$figure "
done

if [[ -n "${OUTPUT_DIR:-}" ]]; then
    mkdir -p -- "$OUTPUT_DIR"
    output=$(cd -- "$OUTPUT_DIR" && pwd)
    if [[ "$output" == "$package_dir" || "$output" == "$package_dir/data" || "$output" == "$package_dir/data/"* ]]; then
        printf 'Use a separate directory for new results.\n' >&2
        exit 2
    fi
else
    output=$(mktemp -d "$package_dir/reproduced.XXXXXX")
fi
printf 'Results: %s\n' "$output"
mkdir -p "$output/data" "$output/plots"

read_metrics() {
    local log=$1
    awk '
        /^Foreground\[0\]/ { split($3, value, "="); goodput = value[2] }
        /^RED\[0\]/ { split($2, value, "="); red1 = value[2] }
        /^RED\[1\]/ { split($2, value, "="); red2 = value[2] }
        /^Wireless drops forward/ { forward = $NF }
        /^Wireless drops reverse/ { reverse = $NF }
        END {
            if (goodput == "" || red1 == "" || red2 == "" ||
                forward == "" || reverse == "") {
                print "Incomplete simulation log: " FILENAME > "/dev/stderr"
                exit 1
            }
            printf "%s\t%s\t%s\t%s\t%s\n", goodput, red1, red2, forward, reverse
        }
    ' "$log"
}

run_case() {
    local figure=$1 rtt=$2 tcp=$3 background=$4 per_pct=$5
    local log="$output/data/figure$figure/$tcp-rtt$rtt-per$per_pct-600s.log"
    local metrics partial
    if [[ -f "$log" ]] && metrics=$(read_metrics "$log" 2>/dev/null); then
        printf 'Reuse %s\n' "$log" >&2
        printf '%s\n' "$metrics"
        return
    fi

    printf 'Figure %s | RTT %s ms | %s | PER %s%%\n' "$figure" "$rtt" "$tcp" "$per_pct" >&2
    partial=$(mktemp "$log.part.XXXXXX")
    if ! (cd "$ns3_dir" && NS_LOG= ./ns3 run "scratch/hybla-experiment --rtt=$rtt --tcp=$tcp --duration=600 --background=$background --per=0.0$per_pct --sack=1 --RngSeed=1 --RngRun=1") >"$partial" 2>&1; then
        tail -n 30 "$partial" >&2
        return 1
    fi
    metrics=$(read_metrics "$partial") || return 1
    mv -f -- "$partial" "$log"
    printf '%s\n' "$metrics"
}

for figure in "${figures[@]}"; do
    mkdir -p "$output/data/figure$figure"
    cp "$package_dir/plots/figure$figure.gnuplot" "$output/plots/"
    summary="$output/data/figure$figure/summary.tsv"
    printf 'RTT_ms\tTCP\tPER_pct\tGoodput_Mbps\tRED_R1\tRED_R2\tDrops_forward\tDrops_reverse\n' > "$summary"
    for rtt in 25 50 100 200 300 450 600; do
        case "$figure" in
            6) protocols=(TcpHybla TcpNewReno); per_values=(0); background=1 ;;
            7) protocols=(TcpHybla TcpNewReno); per_values=(1); background=0 ;;
            8) protocols=(TcpHybla); per_values=(1 5); background=1 ;;
        esac
        for tcp in "${protocols[@]}"; do
            for per_pct in "${per_values[@]}"; do
                metrics=$(run_case "$figure" "$rtt" "$tcp" "$background" "$per_pct")
                printf '%s\t%s\t%s\t%s\n' "$rtt" "$tcp" "$per_pct" "$metrics" >> "$summary"
            done
        done
    done
done

cd "$output"
for figure in 6 7; do
    [[ "$selected" == *" $figure "* ]] || continue
    awk '
        NR == 1 { print "# RTT_ms Hybla_Mbps NewReno_Mbps"; next }
        $2 == "TcpHybla" { hybla[$1] = $4; next }
        $2 == "TcpNewReno" { print $1, hybla[$1], $4 }
    ' "data/figure$figure/summary.tsv" > "data/figure$figure-goodput.dat"
done

if [[ "$selected" == *" 8 "* ]]; then
    baseline="$package_dir/data/figure6-goodput.dat"
    if [[ "$selected" == *" 6 "* ]]; then
        baseline="$output/data/figure6-goodput.dat"
    fi
    printf 'Figure 8, PER 0%% baseline: %s\n' "$baseline"
    awk '
        /^#/ || $1 == "RTT_ms" { next }
        FILENAME == ARGV[1] { per0[$1] = $2; next }
        $3 == 1 { per1[$1] = $4; next }
        $3 == 5 { print $1, per0[$1], per1[$1], $4 }
    ' "$baseline" data/figure8/summary.tsv > data/figure8-goodput.dat
fi

if command -v gnuplot >/dev/null 2>&1; then
    for figure in "${figures[@]}"; do
        gnuplot "plots/figure$figure.gnuplot"
    done
else
    printf 'Install Gnuplot to render the data in %s/data.\n' "$output" >&2
fi
