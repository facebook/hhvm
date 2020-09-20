#!/usr/bin/env bcc-py
#

from __future__ import absolute_import, division, print_function, unicode_literals

from bcc import BPF, USDT, PerfType, PerfHWConfig
from ctypes import c_int
import argparse
import os
import time

parser = argparse.ArgumentParser(
    description="Log latency of signal handling in hhvm")
parser.add_argument("pids",
                    metavar='pid',
                    type=int,
                    nargs='+',
                    help="pid to signal and attach to.")
args = parser.parse_args()

# see tracing_types.h
cflags = [
    "-DNUM_CPUS={}".format(os.cpu_count()),
    "-DHHVM_TRACING_SIGNUM={}".format(43),
    "-DHHVM_TRACING_MAX_STACK_FRAMES={}".format(200),
    "-DHHVM_TRACING_FILE_NAME_MAX={}".format(128),
    "-DHHVM_TRACING_CLASS_NAME_MAX={}".format(128),
    "-DHHVM_TRACING_FUNCTION_MAX={}".format(128),
]

bpf_text = ""
with open("../strobelight_hhvm_structs.h", "r") as structs_file:
    bpf_text += structs_file.read() + "\n"

with open("./strobelight_hhvm_signal_latency.c", "r") as latency_file:
    bpf_text += latency_file.read()

usdts = []
for pid in args.pids:
    usdt = USDT(pid=pid)
    usdt.enable_probe("hhvm_surprise", "on_hhvm_signal_handler")
    print("Enabled tracing on {}\n".format(pid))
    usdts.append(usdt)

bpf = BPF(text=bpf_text,
          usdt_contexts=usdts,
          cflags=cflags)

hhvm_pids = bpf.get_table("hhvm_pids")
for pid in args.pids:
    hhvm_pids[c_int(pid)] = c_int(1)

# A generic perf event for driving the signal
bpf.attach_perf_event(
    ev_type=PerfType.HARDWARE,
    ev_config=PerfHWConfig.CPU_CYCLES,
    fn_name="on_event",
    sample_period=10000000)

try:
    print("Go run `sudo cat /sys/kernel/debug/tracing/trace_pipe`\n")
    print("Running... ^c to exit")
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    pass

dist = bpf.get_table("dist")
dist.print_log2_hist("u sec")
dist.clear()
