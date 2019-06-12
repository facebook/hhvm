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
"${HH_SERVER}" --check "${REPO}" --config max_workers=2
rm -rf "${REPO}"
