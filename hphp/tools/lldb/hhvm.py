# Copyright 2022-present Facebook. All Rights Reserved.

# pyre-strict

"""The main module you load in your .lldbinit."""

import sys

# pyre-fixme[21]: Could not find module `cores`.
import cores

# pyre-fixme[21]: Could not find module `hhbc`.
import hhbc

# pyre-fixme[21]: Could not find module `idx`.
import idx

import lldb

# pyre-fixme[21]: Could not find module `lookup`.
import lookup

# pyre-fixme[21]: Could not find module `nameof`.
import nameof

# pyre-fixme[21]: Could not find module `pretty`.
import pretty

# pyre-fixme[21]: Could not find module `sizeof`.
import sizeof

# pyre-fixme[21]: Could not find module `stack`.
import stack

# pyre-fixme[21]: Could not find module `unit`.
import unit

# pyre-fixme[21]: Could not find module `utils`.
import utils


def __lldb_init_module(debugger: lldb.SBDebugger, _internal_dict) -> None:
    """Load up the commands and pretty printers of each module.

    Arguments:
        debugger: Current debugger object
        internal_dict: Dict for current script session. For internal use by LLDB only.

    Returns:
        None
    """
    top_module = sys.modules[__name__].__name__
    cores.__lldb_init_module(debugger, top_module)
    hhbc.__lldb_init_module(debugger, top_module)
    idx.__lldb_init_module(debugger, top_module)
    lookup.__lldb_init_module(debugger, top_module)
    nameof.__lldb_init_module(debugger, top_module)
    pretty.__lldb_init_module(debugger, top_module)
    sizeof.__lldb_init_module(debugger, top_module)
    stack.__lldb_init_module(debugger, top_module)
    unit.__lldb_init_module(debugger, top_module)
    utils.__lldb_init_module(debugger, top_module)
