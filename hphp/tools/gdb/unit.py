"""
Set the target Unit; used implicitly by some other commands.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

from compatibility import *

import gdb
from gdbutils import *


#------------------------------------------------------------------------------

curunit = None

class UnitCommand(gdb.Command):
    """Set the current translation unit.

Use `unit none` to unset.  Just `unit` displays the current Unit.
"""

    def __init__(self):
        super(UnitCommand, self).__init__('unit', gdb.COMMAND_DATA,
                                          gdb.COMPLETE_NONE, True)

    @errorwrap
    def invoke(self, args, from_tty):
        argv = gdb.string_to_argv(args)
        global curunit

        if len(argv) == 0:
            if curunit is None:
                print('unit: No Unit set.')
            else:
                gdbprint(curunit)
            return

        if len(argv) > 1:
            print('Usage: unit [Unit*|none]')
            return

        if argv[0] == 'none':
            curunit = None
        else:
            unit_type = T('HPHP::Unit').const().pointer()
            curunit = gdb.parse_and_eval(argv[0]).cast(unit_type)

        gdbprint(curunit)

UnitCommand()
