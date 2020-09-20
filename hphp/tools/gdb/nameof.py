#!/usr/bin/env python3

"""
GDB command for printing the names of various objects.
"""

from compatibility import *

import gdb
from gdbutils import *


#------------------------------------------------------------------------------
# `nameof' command.

class NameOfCommand(gdb.Command):
    """Print the name of an HHVM object."""

    def __init__(self):
        super(NameOfCommand, self).__init__('nameof', gdb.COMMAND_DATA)

    @errorwrap
    def invoke(self, args, from_tty):
        try:
            obj = gdb.parse_and_eval(args)
        except gdb.error:
            print('Usage: nameof <object>')
            return

        name = nameof(obj)

        if name is not None:
            print('"' + name + '"')


NameOfCommand()
