#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# FastThrift Optimization Benchmark Harness
#
# Automates perf stat collection before/after each optimization phase.
# Captures: cycles, instructions, IPC, branches, branch-misses, cache-misses,
#           L1-dcache, iTLB, dTLB, and wall-clock times.
#
# Usage:
#   ./perf_harness.sh snapshot <phase_name>     # Capture a snapshot
#   ./perf_harness.sh compare <before> <after>  # Compare two snapshots
#   ./perf_harness.sh baseline                  # Capture baseline (phase_0)
#   ./perf_harness.sh run <phase_name>          # Snapshot + compare vs baseline
#   ./perf_harness.sh report <phase_name>       # Show a single snapshot
#   ./perf_harness.sh history                   # Show all phases chronologically

set -euo pipefail

# =============================================================================
# Configuration
# =============================================================================

FBSOURCE="/data/users/rroeser/fbsource"
BENCH_TARGET="fbcode//thrift/lib/cpp2/fast_thrift/thrift/client/bench:thrift_client_integration_bench"
BUILD_MODE="@fbcode//mode/opt-clang-lto"
RESULTS_DIR="${FBSOURCE}/fbcode/thrift/lib/cpp2/fast_thrift/bench/perf_results"
RUNS_PER_MEASUREMENT=3

# Perf counters to collect
PERF_EVENTS=(
  "cycles"
  "instructions"
  "branches"
  "branch-misses"
  "cache-references"
  "cache-misses"
  "L1-dcache-loads"
  "L1-dcache-load-misses"
  "L1-icache-load-misses"
  "dTLB-loads"
  "dTLB-load-misses"
  "iTLB-loads"
  "iTLB-load-misses"
)

# Benchmark regex patterns — request and response paths measured separately
BENCH_GROUPS=(
  "Rocket_Request"
  "FastThrift_Request"
  "Rocket_Response"
  "FastThrift_Response"
)

# Individual benchmarks for detailed comparison
BENCH_INDIVIDUAL=(
  "Rocket_Request_MinimalMetadata"
  "FastThrift_Request_MinimalMetadata"
  "Rocket_Request_WithHeaders"
  "FastThrift_Request_WithHeaders"
  "Rocket_Response_Success"
  "FastThrift_Response_Success"
  "Rocket_Response_UndeclaredException"
  "FastThrift_Response_UndeclaredException"
  "Rocket_Response_HighConcurrency"
  "FastThrift_Response_HighConcurrency"
  "Rocket_Request_ChainedPayload"
  "FastThrift_Request_ChainedPayload"
)

# =============================================================================
# Helpers
# =============================================================================

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
DIM='\033[2m'
RESET='\033[0m'

log()  { echo -e "${CYAN}[harness]${RESET} $*"; }
warn() { echo -e "${YELLOW}[harness]${RESET} $*"; }
err()  { echo -e "${RED}[harness]${RESET} $*" >&2; }
ok()   { echo -e "${GREEN}[harness]${RESET} $*"; }

join_events() {
  local IFS=","
  echo "${PERF_EVENTS[*]}"
}

ensure_results_dir() {
  mkdir -p "${RESULTS_DIR}"
}

# =============================================================================
# Build
# =============================================================================

build_benchmark() {
  log "Building benchmark in opt-clang-lto..."
  cd "${FBSOURCE}"

  local output
  output=$(buck2 build "${BUILD_MODE}" "${BENCH_TARGET}" --show-output 2>&1 | tail -1)

  BENCH_BINARY=$(echo "${output}" | awk '{print $2}')

  if [[ -z "${BENCH_BINARY}" || ! -f "${FBSOURCE}/${BENCH_BINARY}" ]]; then
    err "Build failed or binary not found. Output:"
    echo "${output}"
    exit 1
  fi

  BENCH_BINARY="${FBSOURCE}/${BENCH_BINARY}"
  ok "Built: ${BENCH_BINARY}"
}

# =============================================================================
# Perf Stat Collection
# =============================================================================

# Run perf stat for a single benchmark regex, output raw counters to file
collect_perf_stat() {
  local regex="$1"
  local outfile="$2"
  local events
  events=$(join_events)

  perf stat -e "${events}" -o "${outfile}" --append \
    -- "${BENCH_BINARY}" --bm_regex="${regex}" 2>/dev/null

  return $?
}

# Parse a perf stat output file into key=value pairs
parse_perf_stat() {
  local infile="$1"
  # Extract counter lines: "  12,345,678  counter-name"
  grep -E '^\s+[0-9,]+\s+\S+' "${infile}" | while read -r line; do
    local value counter
    value=$(echo "${line}" | awk '{print $1}' | tr -d ',')
    counter=$(echo "${line}" | awk '{print $2}')
    echo "${counter}=${value}"
  done
}

# =============================================================================
# Benchmark Timing Collection
# =============================================================================

# Run benchmark and capture folly benchmark output (time/iter, iters/s)
collect_bench_timings() {
  local regex="$1"
  local outfile="$2"

  "${BENCH_BINARY}" --bm_regex="${regex}" 2>/dev/null | \
    grep -E '^\S+' | grep -vE '^=|^$|^\[' > "${outfile}" || true
}

# =============================================================================
# Snapshot
# =============================================================================

take_snapshot() {
  local phase="$1"
  local phase_dir="${RESULTS_DIR}/${phase}"

  ensure_results_dir
  mkdir -p "${phase_dir}"

  build_benchmark

  log "Phase: ${BOLD}${phase}${RESET}"
  log "Collecting ${RUNS_PER_MEASUREMENT} runs per measurement..."

  # Collect benchmark timings (single run, folly handles iteration count)
  log "Collecting benchmark timings..."
  collect_bench_timings "." "${phase_dir}/timings.txt"

  # Collect perf stat for each benchmark group
  for group in "${BENCH_GROUPS[@]}"; do
    local group_file="${phase_dir}/perf_${group}.txt"
    true > "${group_file}"

    log "  perf stat: ${group} (${RUNS_PER_MEASUREMENT} runs)..."
    for _run in $(seq 1 "${RUNS_PER_MEASUREMENT}"); do
      collect_perf_stat "${group}" "${group_file}"
    done
  done

  # Collect perf stat for individual benchmarks (single run each for speed)
  for bench in "${BENCH_INDIVIDUAL[@]}"; do
    local bench_file="${phase_dir}/perf_individual_${bench}.txt"
    true > "${bench_file}"

    log "  perf stat: ${bench}..."
    collect_perf_stat "^${bench}$" "${bench_file}"
  done

  # Save metadata
  cat > "${phase_dir}/metadata.txt" <<EOF
phase=${phase}
timestamp=$(date -Iseconds)
binary=${BENCH_BINARY}
commit=$(cd "${FBSOURCE}" && sl log -r . --template '{node|short}' 2>/dev/null || echo "unknown")
commit_msg=$(cd "${FBSOURCE}" && sl log -r . --template '{desc|firstline}' 2>/dev/null || echo "unknown")
runs_per_measurement=${RUNS_PER_MEASUREMENT}
EOF

  # Generate summary
  generate_summary "${phase}"

  ok "Snapshot saved to ${phase_dir}/"
}

# =============================================================================
# Summary Generation
# =============================================================================

generate_summary() {
  local phase="$1"
  local phase_dir="${RESULTS_DIR}/${phase}"
  local summary="${phase_dir}/summary.txt"

  {
    echo "================================================================="
    echo "  FastThrift Perf Snapshot: ${phase}"
    echo "  $(date)"
    echo "================================================================="
    echo ""

    # Timings
    echo "--- Benchmark Timings ---"
    if [[ -f "${phase_dir}/timings.txt" ]]; then
      cat "${phase_dir}/timings.txt"
    fi
    echo ""

    # Perf stats per group
    for group in "${BENCH_GROUPS[@]}"; do
      local group_file="${phase_dir}/perf_${group}.txt"
      if [[ -f "${group_file}" ]]; then
        echo "--- Perf Stat: ${group} (${RUNS_PER_MEASUREMENT} runs aggregated) ---"
        # Extract median-ish values (last run's counters)
        parse_perf_stat "${group_file}" | sort
        echo ""
      fi
    done

    # Derived metrics
    echo "--- Derived Metrics ---"
    for impl in "Rocket" "FastThrift"; do
      local req_file="${phase_dir}/perf_${impl}_Request.txt"
      local resp_file="${phase_dir}/perf_${impl}_Response.txt"

      for path_file in "${req_file}" "${resp_file}"; do
        if [[ ! -f "${path_file}" ]]; then continue; fi

        local label
        label=$(basename "${path_file}" .txt | sed 's/perf_//')

        local cycles instructions branches branch_misses
        local cache_refs cache_misses l1d_loads l1d_misses

        cycles=$(grep -E '^\s+[0-9,]+\s+cycles' "${path_file}" | tail -1 | awk '{print $1}' | tr -d ',')
        instructions=$(grep -E '^\s+[0-9,]+\s+instructions' "${path_file}" | tail -1 | awk '{print $1}' | tr -d ',')
        branches=$(grep -E '^\s+[0-9,]+\s+branches' "${path_file}" | tail -1 | awk '{print $1}' | tr -d ',')
        branch_misses=$(grep -E '^\s+[0-9,]+\s+branch-misses' "${path_file}" | tail -1 | awk '{print $1}' | tr -d ',')
        cache_refs=$(grep -E '^\s+[0-9,]+\s+cache-references' "${path_file}" | tail -1 | awk '{print $1}' | tr -d ',')
        cache_misses=$(grep -E '^\s+[0-9,]+\s+cache-misses' "${path_file}" | tail -1 | awk '{print $1}' | tr -d ',')
        l1d_loads=$(grep -E '^\s+[0-9,]+\s+L1-dcache-loads' "${path_file}" | tail -1 | awk '{print $1}' | tr -d ',')
        l1d_misses=$(grep -E '^\s+[0-9,]+\s+L1-dcache-load-misses' "${path_file}" | tail -1 | awk '{print $1}' | tr -d ',')

        if [[ -n "${cycles}" && -n "${instructions}" && "${cycles}" -gt 0 ]]; then
          local ipc
          ipc=$(awk "BEGIN { printf \"%.2f\", ${instructions} / ${cycles} }")
          echo "  ${label}: IPC = ${ipc}"
        fi

        if [[ -n "${branches}" && -n "${branch_misses}" && "${branches}" -gt 0 ]]; then
          local bmr
          bmr=$(awk "BEGIN { printf \"%.3f%%\", (${branch_misses} / ${branches}) * 100 }")
          echo "  ${label}: Branch miss rate = ${bmr}"
        fi

        if [[ -n "${cache_refs}" && -n "${cache_misses}" && "${cache_refs}" -gt 0 ]]; then
          local cmr
          cmr=$(awk "BEGIN { printf \"%.2f%%\", (${cache_misses} / ${cache_refs}) * 100 }")
          echo "  ${label}: Cache miss rate = ${cmr}"
        fi

        if [[ -n "${l1d_loads}" && -n "${l1d_misses}" && "${l1d_loads}" -gt 0 ]]; then
          local l1mr
          l1mr=$(awk "BEGIN { printf \"%.2f%%\", (${l1d_misses} / ${l1d_loads}) * 100 }")
          echo "  ${label}: L1d miss rate = ${l1mr}"
        fi
      done
    done

  } > "${summary}"

  cat "${summary}"
}

# =============================================================================
# Compare Two Snapshots
# =============================================================================

compare_snapshots() {
  local before="$1"
  local after="$2"
  local before_dir="${RESULTS_DIR}/${before}"
  local after_dir="${RESULTS_DIR}/${after}"

  if [[ ! -d "${before_dir}" ]]; then
    err "Snapshot '${before}' not found at ${before_dir}"
    exit 1
  fi
  if [[ ! -d "${after_dir}" ]]; then
    err "Snapshot '${after}' not found at ${after_dir}"
    exit 1
  fi

  echo ""
  echo -e "${BOLD}=================================================================${RESET}"
  echo -e "${BOLD}  Comparison: ${before} → ${after}${RESET}"
  echo -e "${BOLD}=================================================================${RESET}"
  echo ""

  # Compare benchmark timings
  echo -e "${BOLD}--- Wall Clock Timings ---${RESET}"
  printf "%-50s %12s %12s %10s\n" "Benchmark" "${before}" "${after}" "Δ"
  printf "%-50s %12s %12s %10s\n" "---------" "------" "-----" "--"

  if [[ -f "${before_dir}/timings.txt" && -f "${after_dir}/timings.txt" ]]; then
    # Parse each benchmark line from before and after
    while IFS= read -r line; do
      local name time_val
      name=$(echo "${line}" | awk '{print $1}')
      time_val=$(echo "${line}" | awk '{print $2}')

      if [[ -z "${name}" || "${name}" == "=="* || "${name}" == "["* ]]; then
        continue
      fi

      local after_time
      after_time=$(grep "^${name} " "${after_dir}/timings.txt" 2>/dev/null | awk '{print $2}' || echo "")

      if [[ -n "${after_time}" && -n "${time_val}" ]]; then
        # Convert time strings to nanoseconds for comparison
        local before_ns after_ns
        before_ns=$(time_to_ns "${time_val}")
        after_ns=$(time_to_ns "${after_time}")

        if [[ "${before_ns}" -gt 0 ]]; then
          local pct
          pct=$(awk "BEGIN { printf \"%+.1f%%\", ((${after_ns} - ${before_ns}) / ${before_ns}) * 100 }")
          local color="${RESET}"
          if [[ "${after_ns}" -lt "${before_ns}" ]]; then
            color="${GREEN}"
          elif [[ "${after_ns}" -gt "${before_ns}" ]]; then
            color="${RED}"
          fi
          printf "%-50s %12s %12s ${color}%10s${RESET}\n" "${name}" "${time_val}" "${after_time}" "${pct}"
        else
          printf "%-50s %12s %12s %10s\n" "${name}" "${time_val}" "${after_time}" "N/A"
        fi
      fi
    done < "${before_dir}/timings.txt"
  fi

  echo ""

  # Compare perf counters per group
  for group in "${BENCH_GROUPS[@]}"; do
    local bf="${before_dir}/perf_${group}.txt"
    local af="${after_dir}/perf_${group}.txt"

    if [[ ! -f "${bf}" || ! -f "${af}" ]]; then continue; fi

    echo -e "${BOLD}--- Perf Counters: ${group} ---${RESET}"
    printf "%-30s %18s %18s %10s\n" "Counter" "${before}" "${after}" "Δ"
    printf "%-30s %18s %18s %10s\n" "-------" "------" "-----" "--"

    for event in "${PERF_EVENTS[@]}"; do
      local bval aval
      bval=$(grep -E "^\s+[0-9,]+\s+${event}\b" "${bf}" | tail -1 | awk '{print $1}' | tr -d ',')
      aval=$(grep -E "^\s+[0-9,]+\s+${event}\b" "${af}" | tail -1 | awk '{print $1}' | tr -d ',')

      if [[ -n "${bval}" && -n "${aval}" && "${bval}" -gt 0 ]]; then
        local pct
        pct=$(awk "BEGIN { printf \"%+.1f%%\", ((${aval} - ${bval}) / ${bval}) * 100 }")
        local color="${RESET}"
        # For most counters, lower is better
        if [[ "${aval}" -lt "${bval}" ]]; then
          color="${GREEN}"
        elif [[ "${aval}" -gt "${bval}" ]]; then
          color="${RED}"
          # Exception: instructions going up might be ok if IPC is better
          if [[ "${event}" == "instructions" ]]; then
            color="${YELLOW}"
          fi
        fi

        local bfmt afmt
        bfmt=$(format_number "${bval}")
        afmt=$(format_number "${aval}")
        printf "%-30s %18s %18s ${color}%10s${RESET}\n" "${event}" "${bfmt}" "${afmt}" "${pct}"
      fi
    done

    # Derived: IPC comparison
    local b_cyc b_ins a_cyc a_ins
    b_cyc=$(grep -E '^\s+[0-9,]+\s+cycles' "${bf}" | tail -1 | awk '{print $1}' | tr -d ',')
    b_ins=$(grep -E '^\s+[0-9,]+\s+instructions' "${bf}" | tail -1 | awk '{print $1}' | tr -d ',')
    a_cyc=$(grep -E '^\s+[0-9,]+\s+cycles' "${af}" | tail -1 | awk '{print $1}' | tr -d ',')
    a_ins=$(grep -E '^\s+[0-9,]+\s+instructions' "${af}" | tail -1 | awk '{print $1}' | tr -d ',')

    if [[ -n "${b_cyc}" && -n "${b_ins}" && -n "${a_cyc}" && -n "${a_ins}" && "${b_cyc}" -gt 0 && "${a_cyc}" -gt 0 ]]; then
      local b_ipc a_ipc
      b_ipc=$(awk "BEGIN { printf \"%.2f\", ${b_ins} / ${b_cyc} }")
      a_ipc=$(awk "BEGIN { printf \"%.2f\", ${a_ins} / ${a_cyc} }")
      local ipc_delta
      ipc_delta=$(awk "BEGIN { printf \"%+.2f\", ${a_ipc} - ${b_ipc} }")
      local color="${RESET}"
      if awk "BEGIN { exit !(${a_ipc} > ${b_ipc}) }"; then
        color="${GREEN}"
      fi
      printf "%-30s %18s %18s ${color}%10s${RESET}\n" "IPC (derived)" "${b_ipc}" "${a_ipc}" "${ipc_delta}"
    fi

    echo ""
  done

  # Verdict
  echo -e "${BOLD}--- Verdict ---${RESET}"
  generate_verdict "${before}" "${after}"
  echo ""
}

# =============================================================================
# Verdict Generation
# =============================================================================

generate_verdict() {
  local before="$1"
  local after="$2"
  local before_dir="${RESULTS_DIR}/${before}"
  local after_dir="${RESULTS_DIR}/${after}"

  local improvements=0
  local regressions=0
  local neutral=0

  # Check FastThrift request path
  for bench in "FastThrift_Request_MinimalMetadata" "FastThrift_Request_WithHeaders"; do
    local bf="${before_dir}/perf_individual_${bench}.txt"
    local af="${after_dir}/perf_individual_${bench}.txt"
    if [[ ! -f "${bf}" || ! -f "${af}" ]]; then continue; fi

    local b_cyc a_cyc
    b_cyc=$(grep -E '^\s+[0-9,]+\s+cycles' "${bf}" | tail -1 | awk '{print $1}' | tr -d ',')
    a_cyc=$(grep -E '^\s+[0-9,]+\s+cycles' "${af}" | tail -1 | awk '{print $1}' | tr -d ',')

    if [[ -n "${b_cyc}" && -n "${a_cyc}" && "${b_cyc}" -gt 0 ]]; then
      local delta
      delta=$(awk "BEGIN { v = ((${a_cyc} - ${b_cyc}) / ${b_cyc}) * 100; printf \"%.1f\", v }")
      if awk "BEGIN { exit !(${delta} < -1.0) }"; then
        echo -e "  ${GREEN}✓${RESET} ${bench}: cycles ${delta}% (improvement)"
        ((improvements++)) || true
      elif awk "BEGIN { exit !(${delta} > 1.0) }"; then
        echo -e "  ${RED}✗${RESET} ${bench}: cycles ${delta}% (regression)"
        ((regressions++)) || true
      else
        echo -e "  ${DIM}≈${RESET} ${bench}: cycles ${delta}% (neutral)"
        ((neutral++)) || true
      fi
    fi
  done

  # Check FastThrift response path
  for bench in "FastThrift_Response_Success" "FastThrift_Response_HighConcurrency"; do
    local bf="${before_dir}/perf_individual_${bench}.txt"
    local af="${after_dir}/perf_individual_${bench}.txt"
    if [[ ! -f "${bf}" || ! -f "${af}" ]]; then continue; fi

    local b_cyc a_cyc
    b_cyc=$(grep -E '^\s+[0-9,]+\s+cycles' "${bf}" | tail -1 | awk '{print $1}' | tr -d ',')
    a_cyc=$(grep -E '^\s+[0-9,]+\s+cycles' "${af}" | tail -1 | awk '{print $1}' | tr -d ',')

    if [[ -n "${b_cyc}" && -n "${a_cyc}" && "${b_cyc}" -gt 0 ]]; then
      local delta
      delta=$(awk "BEGIN { v = ((${a_cyc} - ${b_cyc}) / ${b_cyc}) * 100; printf \"%.1f\", v }")
      if awk "BEGIN { exit !(${delta} < -1.0) }"; then
        echo -e "  ${GREEN}✓${RESET} ${bench}: cycles ${delta}% (improvement)"
        ((improvements++)) || true
      elif awk "BEGIN { exit !(${delta} > 1.0) }"; then
        echo -e "  ${RED}✗${RESET} ${bench}: cycles ${delta}% (regression)"
        ((regressions++)) || true
      else
        echo -e "  ${DIM}≈${RESET} ${bench}: cycles ${delta}% (neutral)"
        ((neutral++)) || true
      fi
    fi
  done

  echo ""
  if [[ "${regressions}" -gt 0 ]]; then
    echo -e "  ${RED}${BOLD}⚠ ${regressions} regression(s) detected. Investigate before proceeding.${RESET}"
  elif [[ "${improvements}" -gt 0 ]]; then
    echo -e "  ${GREEN}${BOLD}✓ ${improvements} improvement(s), ${neutral} neutral. Safe to proceed.${RESET}"
  else
    echo -e "  ${YELLOW}${BOLD}≈ No significant changes detected.${RESET}"
  fi
}

# =============================================================================
# Utility Functions
# =============================================================================

# Convert time strings like "1.10us", "761.63ns", "7.35ms" to nanoseconds
time_to_ns() {
  local val="$1"
  local num unit

  num="${val%%[a-z]*}"
  unit=$(echo "${val}" | grep -oE '[a-z]+$')

  case "${unit}" in
    ns) awk "BEGIN { printf \"%d\", ${num} }" ;;
    us) awk "BEGIN { printf \"%d\", ${num} * 1000 }" ;;
    ms) awk "BEGIN { printf \"%d\", ${num} * 1000000 }" ;;
    s)  awk "BEGIN { printf \"%d\", ${num} * 1000000000 }" ;;
    *)  echo "0" ;;
  esac
}

# Format large numbers with commas: 12345678 -> 12,345,678
format_number() {
  local num="$1"
  echo "${num}" | sed ':a;s/\B[0-9]\{3\}\>$/,&/;ta'
}

# =============================================================================
# History
# =============================================================================

show_history() {
  ensure_results_dir

  echo -e "${BOLD}=================================================================${RESET}"
  echo -e "${BOLD}  FastThrift Optimization History${RESET}"
  echo -e "${BOLD}=================================================================${RESET}"
  echo ""

  local phases=()
  for dir in "${RESULTS_DIR}"/*/; do
    if [[ -f "${dir}/metadata.txt" ]]; then
      phases+=("$(basename "${dir}")")
    fi
  done

  if [[ ${#phases[@]} -eq 0 ]]; then
    warn "No snapshots found in ${RESULTS_DIR}/"
    return
  fi

  printf "%-20s %-25s %-12s %s\n" "Phase" "Timestamp" "Commit" "Description"
  printf "%-20s %-25s %-12s %s\n" "-----" "---------" "------" "-----------"

  for phase in "${phases[@]}"; do
    local meta="${RESULTS_DIR}/${phase}/metadata.txt"
    local ts commit msg
    ts=$(grep '^timestamp=' "${meta}" | cut -d= -f2-)
    commit=$(grep '^commit=' "${meta}" | cut -d= -f2-)
    msg=$(grep '^commit_msg=' "${meta}" | cut -d= -f2-)
    printf "%-20s %-25s %-12s %s\n" "${phase}" "${ts}" "${commit}" "${msg}"
  done

  echo ""

  # If we have baseline + at least one other, show progression
  if [[ ${#phases[@]} -ge 2 ]]; then
    echo -e "${BOLD}--- Progression vs Baseline ---${RESET}"
    local baseline="${phases[0]}"

    for ((i = 1; i < ${#phases[@]}; i++)); do
      local phase="${phases[$i]}"
      echo ""
      echo -e "${CYAN}${baseline} → ${phase}:${RESET}"

      # Quick cycles comparison for key benchmarks
      for bench in "FastThrift_Request_WithHeaders" "FastThrift_Response_Success"; do
        local bf="${RESULTS_DIR}/${baseline}/perf_individual_${bench}.txt"
        local af="${RESULTS_DIR}/${phase}/perf_individual_${bench}.txt"
        if [[ ! -f "${bf}" || ! -f "${af}" ]]; then continue; fi

        local b_cyc a_cyc
        b_cyc=$(grep -E '^\s+[0-9,]+\s+cycles' "${bf}" | tail -1 | awk '{print $1}' | tr -d ',')
        a_cyc=$(grep -E '^\s+[0-9,]+\s+cycles' "${af}" | tail -1 | awk '{print $1}' | tr -d ',')

        if [[ -n "${b_cyc}" && -n "${a_cyc}" && "${b_cyc}" -gt 0 ]]; then
          local pct color
          pct=$(awk "BEGIN { printf \"%+.1f%%\", ((${a_cyc} - ${b_cyc}) / ${b_cyc}) * 100 }")
          if awk "BEGIN { exit !(${a_cyc} < ${b_cyc}) }"; then
            color="${GREEN}"
          else
            color="${RED}"
          fi
          echo -e "  ${bench}: ${color}${pct}${RESET} cycles"
        fi
      done
    done
  fi
}

# =============================================================================
# Report a single snapshot
# =============================================================================

show_report() {
  local phase="$1"
  local summary="${RESULTS_DIR}/${phase}/summary.txt"

  if [[ ! -f "${summary}" ]]; then
    err "Snapshot '${phase}' not found. Run: $0 snapshot ${phase}"
    exit 1
  fi

  cat "${summary}"
}

# =============================================================================
# Main
# =============================================================================

usage() {
  cat <<EOF
FastThrift Optimization Benchmark Harness

Usage:
  $0 snapshot <phase_name>      Capture perf snapshot for a phase
  $0 compare  <before> <after>  Compare two snapshots side-by-side
  $0 baseline                   Capture baseline snapshot (phase_0_baseline)
  $0 run      <phase_name>      Snapshot + auto-compare vs baseline
  $0 report   <phase_name>      Display a saved snapshot
  $0 history                    Show all phases with progression

Examples:
  $0 baseline                              # Before any changes
  $0 run phase_1_serialized_size_zc        # After optimization #1
  $0 run phase_2_headroom_reuse            # After optimization #2
  $0 compare phase_1_serialized_size_zc phase_2_headroom_reuse
  $0 history                               # See cumulative progress

Output:
  Results saved to: ${RESULTS_DIR}/
EOF
}

main() {
  if [[ $# -lt 1 ]]; then
    usage
    exit 1
  fi

  local cmd="$1"
  shift

  case "${cmd}" in
    snapshot)
      if [[ $# -lt 1 ]]; then
        err "Usage: $0 snapshot <phase_name>"
        exit 1
      fi
      take_snapshot "$1"
      ;;

    compare)
      if [[ $# -lt 2 ]]; then
        err "Usage: $0 compare <before> <after>"
        exit 1
      fi
      compare_snapshots "$1" "$2"
      ;;

    baseline)
      take_snapshot "phase_0_baseline"
      ;;

    run)
      if [[ $# -lt 1 ]]; then
        err "Usage: $0 run <phase_name>"
        exit 1
      fi
      local phase="$1"
      take_snapshot "${phase}"

      # Auto-compare vs baseline if it exists
      if [[ -d "${RESULTS_DIR}/phase_0_baseline" && "${phase}" != "phase_0_baseline" ]]; then
        compare_snapshots "phase_0_baseline" "${phase}"
      else
        warn "No baseline found. Run '$0 baseline' first for automatic comparisons."
      fi
      ;;

    report)
      if [[ $# -lt 1 ]]; then
        err "Usage: $0 report <phase_name>"
        exit 1
      fi
      show_report "$1"
      ;;

    history)
      show_history
      ;;

    *)
      err "Unknown command: ${cmd}"
      usage
      exit 1
      ;;
  esac
}

main "$@"
