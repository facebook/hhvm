#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

if [ $# -lt 2 ]; then
  echo -e \
    "Usage: ./$(basename "$0") package_dir install_dir [install_aux_dir]\n" \
    "  where\n" \
    "    package_dir - location where all packages should be download.\n" \
    "    install_dir - location where all packages should be installed.\n" \
    "                  Note: it should already contain a built version of mcrouter in it.\n" \
    "                        i.e. there should be a binary install_dir/bin/mcrouter\n\n" \
    "    install_aux_dir - aux location where all packages should be installed  \n\n" \
    "You can also alter script behavior by setting next environment variables:\n" \
    "  MEMCACHED_PORT - port that would be used by memcached (default: 15000)\n" \
    "  MCROUTER_PORT - port that would be used by mcrouter (default: 15001)\n" \
    "  RUN_DURATION - duration of each individual run in seconds (time during which we \n" \
    "                 collect performance measurements).\n" \
    "                 Note: it doesn't include startup, warmup and cleanup time.\n" \
    "                 (default: 30)\n" \
    "  TWEMPERF_BIN - path to twemperf binary.\n" \
    "                 By default we would download and build a version from github.\n" \
    "  MEMCACHED_BIN - path to memcached binary.\n" \
    "                  By default we would lookup up for installed memcached, if none \n" \
    "                  found, we would download and build it on our own."
  exit 0
fi

# This port will be used for running our own memcached instance.
MEMCACHED_PORT=${MEMCACHED_PORT:-15000}

# This port will be used for running mcrouter instance.
MCROUTER_PORT=${MCROUTER_PORT:-15001}

# Duration of individual workload run.
RUN_DURATION=${RUN_DURATION:-30}

sudo apt-get install -y wget gcc g++ libevent-dev make build-essential

source common.sh

# Disable verbose logging.
set +x

if [ ! -d "$INSTALL_DIR/bin" ]; then
  mkdir "$INSTALL_DIR/bin"
fi

# Ensure that we have twemperf installed.
if [ -z "$TWEMPERF_BIN" ]; then
  echo -n "TWEMPERF_BIN is not set..."
  TWEMPERF_BIN=$INSTALL_DIR/bin/mcperf
  if [ ! -e "$TWEMPERF_BIN" ]; then
    if [ ! -d twemperf ]; then
      echo -n "Downloading source code..."
      git clone https://github.com/twitter/twemperf || die "Failed to clone twemperf"
    else
      echo -n "Found cloned repo..."
    fi

    echo -n "Building it..."

    cd twemperf && autoreconf -i && ./configure && make -j 4 && cd .. || die "Failed to build twemperf"
    ln -s "$PKG_DIR/twemperf/src/mcperf" "$TWEMPERF_BIN" || die "Couldn't install twemperf into $TWEMPERF_BIN"

    echo "Twemperf was installed to $TWEMPERF_BIN"
  else
    echo "Found existing installation of twemperf in $TWEMPERF_BIN"
  fi
else
  echo "TWEMPERF_BIN is set to $TWEMPERF_BIN"
fi

# Ensure that we have memcached installed.
echo -n "Looking for memcached installation..."
if [ -z "$MEMCACHED_BIN" ]; then
  echo -n "MEMCACHED_BIN is not defined..."
  if MEMCACHED_BIN=$(which memcached); then
    echo "Found memcached: $MEMCACHED_BIN"
  else
    MEMCACHED_BIN=$INSTALL_DIR/bin/memcached
    if [ ! -e "$MEMCACHED_BIN" ]; then
      if [ ! -d memcached-1.4.24 ]; then
        echo -n "Downloading source code..."
        wget http://www.memcached.org/files/memcached-1.4.24.tar.gz &> /dev/null || die "Failed to download memcached source code!"
        tar -xvf memcached-1.4.24.tar.gz || die "Failed to extract memcached source code"
      fi
      echo -n "Building it..."

      cd memcached-1.4.24 && ./configure && make all && cd .. || die "Failed to build memcached!"
      ln -s "$PKG_DIR/memcached-1.4.24/memcached" "$MEMCACHED_BIN" || die "Couldn't install memcached into $MEMCACHED_BIN"

      echo "Memcached was installed to $MEMCACHED_BIN"
    else
      echo "Found existing installation of memcached in $MEMCACHED_BIN"
    fi
  fi
else
  echo "MEMCACHED_BIN is set to $MEMCACHED_BIN"
fi

# Check that we have mcrouter.
MCROUTER_BIN=$INSTALL_DIR/bin/mcrouter
if [ ! -e "$MCROUTER_BIN" ]; then
  die "Cannot find mcrouter in install dir, please build and install it before running loadtest!"
fi

# Workload runner, accepts next arguments, runs it for 30 seconds
#  $1 - number of connections.
#  $2 - qps.
#  $3 - method (e.g. get, set, gets, cas, etc.)
function run_workload() {
  # We're goin to start spawning child processes, set up a trap to properly clean them up.
  function cleanup() {
    kill "$MEMCACHED_PID" "$MCROUTER_PID" "$TWEMPERF_PID" &> /dev/null

    # Allow everything to shutdown.
    sleep 2
  }

  trap cleanup EXIT

  # Start memcached.
  "$MEMCACHED_BIN" -t 1 -p "$MEMCACHED_PORT" &> /dev/null &
  MEMCACHED_PID=$!

  # Allow memcached some time to startup.
  sleep 1

  # Check that memcached is running.
  echo version | nc 0 "$MEMCACHED_PORT" | grep "VERSION" &> /dev/null || die "Failed to start memcached!"

  "$MCROUTER_BIN" --config-str="{\"pools\":{\"A\":{\"servers\":[\"localhost:$MEMCACHED_PORT\"],\"server_timeout\":1000}},\"route\":\"Pool|A\"}" -p "$MCROUTER_PORT" &> /dev/null &
  MCROUTER_PID=$!

  # Allow mcrouter some time to startup.
  sleep 1

  # Check that mcrouter is running.
  echo version | nc 0 "$MCROUTER_PORT" | grep "mcrouter" &> /dev/null || die "Failed to start mcrouter!"

  # Calculate total number of calls to do.
  let "NUM_CALLS = $2 * ($RUN_DURATION + 6) / $1"

  "$TWEMPERF_BIN" --server=localhost --port="$MCROUTER_PORT" --num-conns="$1" --call-rate="$2" --num-calls="$NUM_CALLS" --method="$3" &> /dev/null &
  TWEMPERF_PID=$!

  # Wait for a couple of seconds.
  sleep 3

  # Collect CPU usage of mcrouter.
  PIDSTAT_OUT=$(pidstat -h -p "$MCROUTER_PID" "$RUN_DURATION" 1 | grep -v Linux | grep -v '#' | grep -v -e '^$' | awk '{print $7}')

  echo "$PIDSTAT_OUT"
}

echo "**************************************************************************"
echo "**************************************************************************"

echo -n "Running workload (num connections = 100, qps = 20000, method = set)... "
echo " Mcrouter CPU usage: $(run_workload 100 20000 set)%"

echo -n "Running workload (num connections = 100, qps = 40000, method = set)... "
echo " Mcrouter CPU usage: $(run_workload 100 40000 set)%"

echo -n "Running workload (num connections = 100, qps = 80000, method = set)... "
echo " Mcrouter CPU usage: $(run_workload 100 80000 set)%"

echo -n "Running workload (num connections = 100, qps = 20000, method = get)... "
echo " Mcrouter CPU usage: $(run_workload 100 20000 get)%"

echo -n "Running workload (num connections = 100, qps = 40000, method = get)... "
echo " Mcrouter CPU usage: $(run_workload 100 40000 get)%"

echo -n "Running workload (num connections = 100, qps = 80000, method = get)... "
echo " Mcrouter CPU usage: $(run_workload 100 80000 get)%"
