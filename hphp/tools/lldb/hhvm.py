#!/usr/bin/env python3

import lldb

class WalkstkCommand:
    def __init__(self, debugger, internal_dict):
        pass
    def __call__(self, debugger, command, exe_ctx, result):
        pass
    def get_short_help(self):
        return "Traverse the interleaved VM and native stacks"
    def get_long_help(self):
        return """\
The output backtrace has the following format:

    #<bt frame> <fp> @ <rip>: <function> [at <filename>:<line>]

`walkstk' depends on the custom HHVM unwinder defined in unwind.py."""

def __lldb_init_module(debugger, dict):
    debugger.HandleCommand('command script add -c hhvm.WalkstkCommand "walkstk"')
