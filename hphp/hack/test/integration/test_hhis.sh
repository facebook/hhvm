#!/bin/bash
set -ex
HH_SERVER="$1"
if [ ! -x "${HH_SERVER}" ]; then
  echo "Usage: $0 /path/to/hh_server"
  exit 1
fi
REPO="$(mktemp -d)"
cat > "${REPO}/.hhconfig" <<EOF
unsafe_rx=false
EOF
export HH_TEST_MODE=1  # avoid writing a bunch of telemetry
set +e
"${HH_SERVER}" --check "${REPO}" --config max_workers=2
code=$?
if [[ "$code" == 126 ]]; then
  # 126 means "not executable", typically in buck2 from "Text file is busy" because there are still open FD write handles to the binary
  # Ugly workaround for now: https://www.internalfb.com/intern/qa/312685/text-file-is-busy---test-is-run-before-fclose-on-e
  # This workaround should be removed once T107518211 is closed.
  sleep 20
  "${HH_SERVER}" --check "${REPO}" --config max_workers=2
fi
rm -rf "${REPO}"
