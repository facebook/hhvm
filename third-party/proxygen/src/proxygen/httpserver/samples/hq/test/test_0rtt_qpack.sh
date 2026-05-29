#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

# Test script to verify that 0-RTT connections use the QPACK dynamic table.
#
# Flow:
#   1. Build the HQ binary
#   2. Start a server
#   3. First client connection (full handshake) — gets session ticket + SETTINGS
#   4. Second client connection (0-RTT) — should use cached SETTINGS to enable
#      the QPACK dynamic table immediately, before receiving server SETTINGS
#   5. Verify via logs:
#      a. 0-RTT was used (qlog: "used_zero_rtt": true)
#      b. Client applied cached SETTINGS with tableSize > 0 at onTransportReady
#      c. QPACK encoder was configured with non-zero table size
#      d. QPACK encoder made dynamic table insertions (encoding name index)

set -euo pipefail

TMPDIR=$(mktemp -d /tmp/hq_0rtt_test.XXXXXX)
PSK_FILE="$TMPDIR/psk_cache"
QLOG_DIR="$TMPDIR/qlog"
SERVER_LOG="$TMPDIR/server.log"
CLIENT1_LOG="$TMPDIR/client1.log"
CLIENT2_LOG="$TMPDIR/client2.log"
PASSED=0
FAILED=0

cleanup() {
  if [[ -n "${SERVER_PID:-}" ]]; then
    kill "$SERVER_PID" 2>/dev/null || true
    wait "$SERVER_PID" 2>/dev/null || true
  fi
  if (( FAILED == 0 )); then
    rm -rf "$TMPDIR"
  else
    echo "Test artifacts preserved in $TMPDIR"
  fi
}
trap cleanup EXIT

pass() {
  echo "  PASS: $1"
  ((PASSED++)) || true
}

fail() {
  echo "  FAIL: $1"
  ((FAILED++)) || true
}

check() {
  local desc="$1"
  shift
  if "$@"; then
    pass "$desc"
  else
    fail "$desc"
  fi
}

echo "=== Building HQ binary ==="
HQ_BIN=$(buck build //proxygen/httpserver/samples/hq:hq --show-full-output 2>/dev/null \
  | awk '{print $2}')
if [[ ! -x "$HQ_BIN" ]]; then
  echo "FATAL: Failed to build HQ binary"
  exit 1
fi
echo "Binary: $HQ_BIN"

mkdir -p "$QLOG_DIR"

# Pick a random port to avoid conflicts
PORT=$((10000 + RANDOM % 50000))

echo ""
echo "=== Starting server on port $PORT ==="
$HQ_BIN --mode server --port "$PORT" --host ::1 \
  --v=3 \
  2>"$SERVER_LOG" &
SERVER_PID=$!
sleep 2

if ! kill -0 "$SERVER_PID" 2>/dev/null; then
  echo "FATAL: Server failed to start"
  cat "$SERVER_LOG"
  exit 1
fi
echo "Server PID: $SERVER_PID"

echo ""
echo "=== Connection 1: full handshake (obtaining session ticket) ==="
$HQ_BIN --mode client --host ::1 --port "$PORT" --path / \
  --early_data --psk_file "$PSK_FILE" \
  --v=3 \
  2>"$CLIENT1_LOG" || true

# Wait for PSK file to sync to disk (PersistentQuicPskCache has 1s sync interval)
sleep 2

echo ""
echo "=== Connection 2: 0-RTT with cached SETTINGS ==="
# Use --vmodule to get QPACK encoder detail (level 10) and H3EarlyDataHandler
# detail (level 4), without flooding everything else.
$HQ_BIN --mode client --host ::1 --port "$PORT" --path / \
  --early_data --psk_file "$PSK_FILE" \
  --qlogger_path "$QLOG_DIR" \
  --v=4 \
  --vmodule="QPACKEncoder=10,QPACKCodec=6" \
  2>"$CLIENT2_LOG" || true

echo ""
echo "=== Results ==="

# 1. Verify PSK file was created (session ticket was received)
check "PSK file created after first connection" \
  test -s "$PSK_FILE"

# 2. Check qlog for 0-RTT
QLOG_FILE=$(find "$QLOG_DIR" -name '*.qlog' -o -name '*.json' 2>/dev/null | head -1)
if [[ -n "$QLOG_FILE" ]]; then
  check "0-RTT used (qlog: used_zero_rtt)" \
    grep -q '"used_zero_rtt": true' "$QLOG_FILE"
else
  fail "0-RTT used (qlog) — no qlog file found"
fi

# 3. Check that cached settings applied with non-zero table size
#    On 0-RTT the client calls applySettings twice:
#      First from cached ticket (at onTransportReady)
#      Second from server SETTINGS frame
FIRST_TABLE_SIZE=$(grep "Applied SETTINGS" "$CLIENT2_LOG" | head -1 \
  | grep -oP 'tableSize=\K[0-9]+' || echo "0")
check "Cached SETTINGS applied with tableSize=$FIRST_TABLE_SIZE (want > 0)" \
  test "$FIRST_TABLE_SIZE" -gt 0

# 4. Check that the QPACK encoder table size was set to non-zero
check "QPACK encoder configured with non-zero table size" \
  grep -q "setEncoderHeaderTableSize size=[1-9]" "$CLIENT2_LOG"

# 5. Check that QPACK encoder actually inserted entries into the dynamic table
#    "encoding name index=" at VLOG(10) means a dynamic table reference was used
check "QPACK encoder used dynamic table (encoding name index)" \
  grep -q "encoding name index=" "$CLIENT2_LOG"

# 6. Verify replay safe was signaled (confirms 0-RTT completed)
check "Transport replay safe signaled" \
  grep -q "replay safe" "$CLIENT2_LOG"

echo ""
echo "=== Summary ==="
echo "Passed: $PASSED"
echo "Failed: $FAILED"
if (( FAILED > 0 )); then
  echo ""
  echo "--- Client 1 log ---"
  cat "$CLIENT1_LOG"
  echo ""
  echo "--- Client 2 log ---"
  cat "$CLIENT2_LOG"
  echo ""
  echo "--- Server log (last 50 lines) ---"
  tail -50 "$SERVER_LOG"
  exit 1
fi
