#!/bin/bash
# Client burst for proxygen_error_response_race.php.
#   $1 port  $2 processes  $3 sockets-per-process  $4 blast time (epoch ns)  $5 result dir
# Pure bash /dev/tcp so the burst costs a few MB per process instead of a whole
# HHVM. The arrivals have to be close together, hence the spin barrier.
port=$1; nprocs=$2; per=$3; blast_ns=$4; outdir=$5

# Resolve localhost once. Each /dev/tcp to a name costs a lookup, and at this
# socket count that widens the arrivals past the 10ms timer-wheel tick the test
# depends on. A literal also keeps this working where loopback is v6-only.
addr=$(getent ahosts localhost 2>/dev/null | awk 'NR==1{print $1}')
[ -n "$addr" ] || addr=localhost

req=$'POST /timeout_race_slow.php?ms=1 HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/octet-stream\r\nContent-Length: 1000000\r\n\r\n'

for p in $(seq 1 "$nprocs"); do
  (
    fds=()
    for _ in $(seq 1 "$per"); do
      if exec {fd}<>/dev/tcp/"$addr"/"$port" 2>/dev/null; then fds+=("$fd"); fi
    done
    while (( $(date +%s%N) < blast_ns )); do :; done
    for fd in "${fds[@]}"; do printf '%s' "$req" >&"$fd"; done
    # One body byte, separately: ingress EOM never arrives, so the idle timer
    # stays armed.
    for fd in "${fds[@]}"; do printf 'x' >&"$fd"; done
    ans=0; drop=0
    for fd in "${fds[@]}"; do
      if read -r -t 30 line <&"$fd" && [[ $line == HTTP/1.1* ]]; then
        ans=$((ans+1))
      else
        drop=$((drop+1))
      fi
    done
    echo "$ans $drop" > "$outdir/$p"
  ) &
done
wait
