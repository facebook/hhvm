# Copyright 2022-present Facebook. All Rights Reserved.

# pyre-unsafe

"""The main module you load in your .lldbinit."""

import sys

# pyre-fixme[21]: Could not find module `cores`.
import cores

# pyre-fixme[21]: Could not find module `hhbc`.
import hhbc

# pyre-fixme[21]: Could not find module `idx`.
import idx

# pyre-fixme[21]: Could not find module `lldb`.
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


def __lldb_init_module(debugger, internal_dict):
    """Load up the commands and pretty printers of each module.

    Arguments:
        debugger: Current debugger object
        internal_dict: Dict for current script session. For internal use by LLDB only.

    Returns:
        None
    """
    top = sys.modules[__name__].__name__
    cores.__lldb_init_module(debugger, internal_dict, top)
    hhbc.__lldb_init_module(debugger, internal_dict, top)
    idx.__lldb_init_module(debugger, internal_dict, top)
    lookup.__lldb_init_module(debugger, internal_dict, top)
    nameof.__lldb_init_module(debugger, internal_dict, top)
    pretty.__lldb_init_module(debugger, internal_dict, top)
    sizeof.__lldb_init_module(debugger, internal_dict, top)
    stack.__lldb_init_module(debugger, internal_dict, top)
    unit.__lldb_init_module(debugger, internal_dict, top)
    utils.__lldb_init_module(debugger, internal_dict, top)
