#!/usr/bin/env bcc-py
#

#
#  An example of using kSignumAll to profile all active requests
#
# This sets up listeners for all provided HHVM pids, and then sends
# the signal to each pid when you press enter. It then dumps and stack
# traces it receives out to the console.
#

from __future__ import absolute_import, division, print_function, unicode_literals

from bcc import BPF, USDT
import ctypes as ct
import argparse
import os

parser = argparse.ArgumentParser(
    description="Trigger dumps of current HHVM stack traces by sending signals")
parser.add_argument("pids",
                    metavar='pid',
                    type=int,
                    nargs='+',
                    help="pid to signal and attach to.")
args = parser.parse_args()

# see tracing_types.h
cflags = [
    "-DNUM_CPUS={}".format(os.cpu_count()),
    "-DHHVM_TRACING_MAX_STACK_FRAMES={}".format(200),
    "-DHHVM_TRACING_FILE_NAME_MAX={}".format(128),
    "-DHHVM_TRACING_CLASS_NAME_MAX={}".format(128),
    "-DHHVM_TRACING_FUNCTION_MAX={}".format(128),
]

bpf_text = ""
with open("../strobelight_hhvm_structs.h", "r") as structs_file:
    bpf_text += structs_file.read() + "\n"

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
    input("Press Enter to signal hhvm...")
    for pid in args.pids:
        os.kill(pid, 42)
    try:
        b.perf_buffer_poll(3000)
    except KeyboardInterrupt:
        exit()
