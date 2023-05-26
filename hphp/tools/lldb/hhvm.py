# Copyright 2022-present Facebook. All Rights Reserved.

""" The main module you load in your .lldbinit. """


import lldb
import sys

import idx
import lookup
import nameof
import pretty
import sizeof
import stack


def __lldb_init_module(debugger, internal_dict):
    """ Load up the commands and pretty printers of each module.
    
    Arguments:
        debugger: Current debugger object
        internal_dict: Dict for current script session. For internal use by LLDB only.

    Returns:
        None
    """
    top = sys.modules[__name__].__name__
    idx.__lldb_init_module(debugger, internal_dict, top)
    lookup.__lldb_init_module(debugger, internal_dict, top)
    nameof.__lldb_init_module(debugger, internal_dict, top)
    pretty.__lldb_init_module(debugger, internal_dict, top)
    sizeof.__lldb_init_module(debugger, internal_dict, top)
    stack.__lldb_init_module(debugger, internal_dict, top)
