"""
GDB convenience functions for unpacking types.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

import gdb
from gdbutils import *


#------------------------------------------------------------------------------
# `ptr' function.

class PtrFunction(gdb.Function):
    def __init__(self):
        super(PtrFunction, self).__init__('ptr')

    def invoke(self, val):
        return rawptr(val)

PtrFunction._PtrFunction__doc__ = rawptr.__doc__

PtrFunction()


#------------------------------------------------------------------------------
# `deref' function.

class DerefFunction(gdb.Function):
    def __init__(self):
        super(DerefFunction, self).__init__('deref')

    def invoke(self, val):
        return deref(val)

DerefFunction._DerefFunction__doc__ = deref.__doc__

DerefFunction()
