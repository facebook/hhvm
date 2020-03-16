#!/usr/bin/env bcc-py
#

from __future__ import absolute_import, division, print_function, unicode_literals

from bcc import BPF, USDT, PerfType, PerfHWConfig
import ctypes as ct
import argparse
import os

parser = argparse.ArgumentParser(
    description="Log stack snapshots triggered by HW events")
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

with open("./strobelight_hhvm_signal.c", "r") as signal_file:
    bpf_text += signal_file.read() + "\n"

with open("../strobelight_hhvm_stacks_usdt_probe.c", "r") as stacks_file:
    bpf_text += stacks_file.read() + "\n"

usdts = []
for pid in args.pids:
    usdt = USDT(pid=pid)
    usdt.enable_probe("hhvm_stack", "on_hhvm_event_hook")
    print("Enabled tracing on {}\n".format(pid))
    usdts.append(usdt)

b = BPF(text=bpf_text,
        usdt_contexts=usdts,
        cflags=cflags)

# Track pids to fire signals on
hhvm_pids = b.get_table("hhvm_pids")
for pid in args.pids:
    hhvm_pids[ct.c_int(pid)] = ct.c_int(1)

# Collect a stack every 10m cycles
b.attach_perf_event(
    ev_type=PerfType.HARDWARE,
    ev_config=PerfHWConfig.CPU_CYCLES,
    fn_name="on_event",
    sample_period=10000000)


class HackSymbol():
    '''Struct from strobelight_hhvm_structs.h'''
    def __init__(self, bcc_obj):
        self.line = bcc_obj.line
        encoding = 'utf-8'
        self.file_name = bcc_obj.file_name.decode(encoding)
        self.class_name = bcc_obj.class_name.decode(encoding)
        self.function = bcc_obj.function.decode(encoding)

    def __str__(self):
        return "{}:{} {}::{}".format(self.file_name,
                                     self.line,
                                     self.class_name,
                                     self.function)


class HackSymbol(ct.Structure):
    '''Struct from strobelight_hhvm_structs.h'''
    _fields_ = [
        ("line", ct.c_int),
        ("file_name", ct.c_char_p),
        ("class_name", ct.c_char_p),
        ("function", ct.c_char_p),
    ]

    def __str__(self):
        encoding = 'utf-8'
        if (self.class_name):
            return '{}:{} {}::{}'.format(self.file_name.decode(encoding),
                                         self.line,
                                         self.class_name.decode(encoding),
                                         self.function.decode(encoding))
        else:
            return '{}:{} {}'.format(self.file_name.decode(encoding),
                                     self.line,
                                     self.function.decode(encoding))


class HackSample(ct.Structure):
    '''Struct from strobelight_hhvm_structs.h'''
    _fields_ = [
        ("pid", ct.c_int),
        ("tid", ct.c_int),
        ("stack_len", ct.c_int),
        ("stack", ct.c_int * 128),
    ]


def print_stack_sample(cpu, data, size):
    sample = ct.cast(data, ct.POINTER(HackSample)).contents
    hack_syms = b.get_table("hack_symbols_map")

    inv_map = {v.value: k for k, v in hack_syms.items()}
    print("\nPID: {} TID: {} Len: {}".format(sample.pid, sample.tid, sample.stack_len))

    for i in range(sample.stack_len):
        sym = inv_map[sample.stack[i]]
        h_sym = HackSymbol(
            line=sym.line,
            file_name=sym.file_name,
            class_name=sym.class_name,
            function=sym.function,
        )
        print("{}=>{}".format(i, h_sym))


b["hack_samples"].open_perf_buffer(print_stack_sample)
while 1:
    try:
        b.perf_buffer_poll()
    except KeyboardInterrupt:
        exit()
