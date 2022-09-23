# Copyright 2022-present Facebook. All Rights Reserved

import optparse
import utils

class WalkstkCommand(utils.Command):
    command = "walkstk"

    @classmethod
    def create_options(cls):
        parser = optparse.OptionParser()
        return parser

    def __init__(self, debugger, internal_dict):
        pass
    def __call__(self, debugger, command, exe_ctx, result):
        pass
    def get_short_help(self):
        return "Traverse the interleaved VM and native stacks"
    def get_long_help(self):
        return f"""\
The output backtrace has the following format:

    #<bt frame> <fp> @ <rip>: <function> [at <filename>:<line>]

{self.command} depends on the custom HHVM unwinder defined in unwind.py."""


def __lldb_init_module(debugger, _internal_dict, top_module=""):
    """ Register the stack-related commands in this file with the LLDB debugger.

    Defining this in this module (in addition to the main hhvm module) allows
    this script to be imported into LLDB separately; LLDB looks for a function with
    this name as module load time.

    Arguments:
        debugger: Current debugger object
        _internal_dict: Dict for current script session. For internal use by LLDB only.

    Returns:
        None
    """
    WalkstkCommand.register_lldb_command(debugger, __name__, top_module)
