"""
GDB convenience function for dereferencing arbitrary pointery types.
"""
# @lint-avoid-python-3-compatibility-imports

import gdb
from gdbutils import *


#------------------------------------------------------------------------------
# `deref' function.

class DerefFunction(gdb.Function):
    def __init__(self):
        super(DerefFunction, self).__init__('deref')

    def invoke(self, val):
        return deref(val)

DerefFunction._DerefFunction__doc__ = deref.__doc__

DerefFunction()
