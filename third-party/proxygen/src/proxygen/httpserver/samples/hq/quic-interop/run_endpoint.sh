#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

# Extra debugging ?
set -x
set -o nounset

VERSION=1
HQ_CLI=/proxygen/_build/proxygen/bin/hq
PORT=443
LOGLEVEL=2

# Set up the routing needed for the simulation
/setup.sh

# Unless noted otherwise, test cases use HTTP/0.9 for file transfers.
PROTOCOL="hq-interop"
HTTPVERSION="0.9"

# Default enormous flow control.

CONN_FLOW_CONTROL="107374182"
STREAM_FLOW_CONTROL="107374182"
INVOCATIONS=$(echo "${REQUESTS}" | tr " " "\n" | awk -F '/' '{ print "/" $4 }' | paste -sd',')
EARLYDATA="false"
PSK_FILE="" # in memory psk
if [ -n "${TESTCASE}" ]; then
    case "${TESTCASE}" in
        "handshake") ;;
        "multiconnect") ;;
        "transfer")
            STREAM_FLOW_CONTROL="262144"
            CONN_FLOW_CONTROL="2621440"
            ;;
        "retry")
            exit 127
            ;;
        "throughput")
            LOGLEVEL=1
        ;;
        "resumption")
            INVOCATIONS=$(${INVOCATIONS//,/ })
            PSK_FILE="/psk"
        ;;
        "zerortt")
            INVOCATIONS=$(${INVOCATIONS//,/ })
            PSK_FILE="/psk"
            EARLYDATA="true"
        ;;
        "http3")
            PROTOCOL="h3"
            HTTPVERSION="1.1"
            ;;
        *)
            exit 127
            ;;
    esac
fi

if [ "${ROLE}" == "client" ]; then
    # Wait for the simulator to start up.
    /wait-for-it.sh sim:57832 -s -t 10
    echo "Starting QUIC client..."
    if [ -n "${REQUESTS}" ]; then
        REQS=("${REQUESTS}")
        REQ=${REQS[0]}
        SERVER=$(echo "$REQ" | cut -d'/' -f3 | cut -d':' -f1)

        for INVOCATION in ${INVOCATIONS}; do

          echo "requesting files '${INVOCATION}'"
          ${HQ_CLI} \
              --mode=client \
              --host="${SERVER}" \
              --port=${PORT} \
              --protocol=${PROTOCOL} \
              --httpversion=${HTTPVERSION} \
              --use_version=true \
              --quic-version=${VERSION} \
              --path="${INVOCATION}" \
              --early_data=${EARLYDATA} \
              --psk_file=${PSK_FILE} \
              --conn_flow_control=${CONN_FLOW_CONTROL} \
              --stream_flow_control=${STREAM_FLOW_CONTROL} \
              --outdir=/downloads \
              --logdir=/logs \
              --qlogger_path=/logs \
              --v=${LOGLEVEL} 2>&1 | tee /logs/client.log
        done
        # This is the best way to troubleshoot.
        # Just uncomment the line below, run the test, then enter containers with
        # docker exec -it [client|server|sim] /bin/bash
        #/bin/bash
    fi

elif [ "$ROLE" == "server" ]; then
    echo "Running QUIC server on [::]:${PORT}"
    ${HQ_CLI} \
        --mode=server \
        --cert=/certs/cert.pem \
        --key=/certs/priv.key \
        --conn_flow_control=${CONN_FLOW_CONTROL} \
        --stream_flow_control=${STREAM_FLOW_CONTROL} \
        --port=${PORT} \
        --httpversion=${HTTPVERSION} \
        --h2port=${PORT} \
        --static_root=/www \
        --logdir=/logs \
        --qlogger_path=/logs \
        --host=:: \
        --congestion=bbr \
        --pacing=true \
        --v=${LOGLEVEL} 2>&1 | tee /logs/server.log
fi
