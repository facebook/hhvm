"""
GDB utility convenience functions.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

from compatibility import *

import gdb
from gdbutils import *


#------------------------------------------------------------------------------
# `ptr' function.

class PtrFunction(gdb.Function):
    def __init__(self):
        super(PtrFunction, self).__init__('ptr')

    @errorwrap
    def invoke(self, val):
        return rawptr(val)

PtrFunction._PtrFunction__doc__ = rawptr.__doc__

PtrFunction()


#------------------------------------------------------------------------------
# `deref' function.

class DerefFunction(gdb.Function):
    def __init__(self):
        super(DerefFunction, self).__init__('deref')

    @errorwrap
    def invoke(self, val):
        return deref(val)

DerefFunction._DerefFunction__doc__ = deref.__doc__

DerefFunction()


#------------------------------------------------------------------------------
# `strhash' function.

class StrhashFunction(gdb.Function):
    """Return the hash of a string."""

    def __init__(self):
        super(StrhashFunction, self).__init__('strhash')

    @errorwrap
    def invoke(self, val):
        return strinfo(val)['hash']

StrhashFunction()


#------------------------------------------------------------------------------
# `hhcry' command.

class HHCryCommand(gdb.Command):
    """Perform a bunch of obscure actions for unspecified reasons."""

    def __init__(self):
        super(HHCryCommand, self).__init__('hhcry', gdb.COMMAND_SUPPORT)

    @errorwrap
    def invoke(self, args, from_tty):
        invalidate_all_memoizers()

HHCryCommand()
