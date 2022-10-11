# Copyright 2022-present Facebook. All Rights Reserved

import utils

class WalkstkCommand(utils.Command):
    command = "walkstk"
    description = "Traverse the interleaved VM and native stacks"
    epilog = """\
The output backtrace has the following format:

    #<bt frame> <fp> @ <rip>: <function> [at <filename>:<line>]

`walkstk` depends on the custom HHVM unwinder defined in unwind.py.
"""

    @classmethod
    def create_parser(cls):
        return cls.default_parser()

    def __init__(self, debugger, internal_dict):
        super().__init__(debugger, internal_dict)

    def __call__(self, debugger, command, exe_ctx, result):
        pass


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
