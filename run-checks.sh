#!/usr/bin/env bash
set -euo pipefail
export LC_ALL=C

if (( $# != 1 )); then
    printf 'Usage: bash run-checks.sh /path/to/ns-3.49\n' >&2
    exit 2
fi
project_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
ns3_dir=$(cd -- "$1" && pwd)
for name in hybla-experiment hybla-config-test hybla-rho-test hybla-figure5; do
    if [[ ! -x "$ns3_dir/ns3" || ! -f "$ns3_dir/scratch/$name.cc" ]]; then
        printf 'Install the project sources first (see README.md).\n' >&2
        exit 2
    fi
done

out=$(mktemp -d "$project_dir/reproduced.checks.XXXXXX")
mkdir -p "$out"/{basic,ack25,cwnd-startup,ssthresh600,mss1024}
printf 'Results: %s\n' "$out"

run_case() {
    local label=$1 log_config=$2 command=$3
    local log="$out/$label.log"
    printf '%s\n' "$label"
    if [[ "$command" == scratch/hybla-experiment\ * ]]; then
        command+=" --RngSeed=1 --RngRun=1"
    fi
    if ! (cd "$ns3_dir" && NS_LOG="$log_config" ./ns3 run "$command") >"$log" 2>&1; then
        tail -n 25 "$log" >&2
        return 1
    fi
}

run_case basic/config '' 'scratch/hybla-config-test'
run_case basic/rho '' 'scratch/hybla-rho-test'
run_case basic/figure5 '' 'scratch/hybla-figure5 --rtt=100'

experiment='scratch/hybla-experiment --sack=1'
congestion="$experiment --background=1 --per=0"
hybla_log='TcpHybla=info|debug|prefix_time'
newreno_log='TcpCongestionOps=level_info|prefix_time'

# DelAckCount applies to all six connections.
for tcp in TcpHybla TcpNewReno; do
    case_args="$congestion --rtt=25 --tcp=$tcp --duration=600"
    run_case "ack25/default-$tcp" '' "$case_args"
    run_case "ack25/ack1-$tcp" '' "$case_args --ns3::TcpSocket::DelAckCount=1"
done
run_case ack25/hybla-rho25 "$hybla_log" "$congestion --rtt=25 --tcp=TcpHybla --duration=5"

for rtt in 25 300 600; do
    run_case "cwnd-startup/hybla-rtt$rtt" "$hybla_log" "$congestion --rtt=$rtt --tcp=TcpHybla --duration=30"
done

# InitialSlowStartThreshold also applies to all six connections.
case_args="$congestion --rtt=600 --tcp=TcpHybla --duration=600"
run_case ssthresh600/default "$hybla_log" "$case_args"
run_case ssthresh600/ssthresh750k "$hybla_log" "$case_args --ns3::TcpSocket::InitialSlowStartThreshold=750000"

for rtt in 25 300; do
    run_case "mss1024/newreno-rtt$rtt" "$newreno_log" "$experiment --rtt=$rtt --tcp=TcpNewReno --duration=600 --background=0 --per=0.01 --ns3::TcpSocket::SegmentSize=1024"
done

for log in "$out"/cwnd-startup/*.log "$out"/ssthresh600/*.log "$out"/mss1024/*.log; do
    if ! grep -Fq 'In SlowStart, updated to cwnd' "$log"; then
        printf 'Missing Slow Start trace in %s. Use an ns-3 build with logging enabled.\n' "$log" >&2
        exit 1
    fi
done
if ! grep -Fq 'Calculated rho=' "$out/ack25/hybla-rho25.log"; then
    printf 'Missing rho trace. Use an ns-3 build with logging enabled.\n' >&2
    exit 1
fi
