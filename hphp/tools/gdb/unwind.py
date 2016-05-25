"""
GDB commands related to the HHVM stack.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

from compatibility import *

import gdb
from gdb.unwinder import Unwinder, register_unwinder
from gdbutils import *
import frame


#------------------------------------------------------------------------------
# HHVM unwinder.

class FrameId(object):
    def __init__(self, sp, ip):
        self.sp = sp
        self.pc = ip

class HHVMUnwinder(Unwinder):
    """Custom unwinder for jitted frames.

Chases the frame pointer chain in frames which appear to be in the TC, and
falls back to the default GDB unwinder(s) otherwise.
"""

    def __init__(self):
        super(HHVMUnwinder, self).__init__('hhvm_unwinder')

    def __call__(self, pending_frame):
        fp = pending_frame.read_register('rbp')
        sp = pending_frame.read_register('rsp')
        ip = pending_frame.read_register('rip')

        if not frame.is_jitted(fp, ip):
            return None

        # GDB wants a FrameId to be a pair of (stack pointer, starting PC) for
        # a given function frame.  Unfortunately, we can't restore the stack
        # pointer correctly in the TC, and we don't know our starting IP.
        #
        # Instead, we just use the stack pointer value for our most recent call
        # into native code, along with the current PC.  It turns out that this
        # is good enough.
        #
        # GDB expects stack pointers to be monotonically nondecreasing as we
        # unwind, so we can't use, e.g., the frame pointer as part of the ID.
        unwind_info = pending_frame.create_unwind_info(FrameId(sp, ip))

        # Restore the saved frame pointer and instruction pointer.
        fp = fp.cast(T('uintptr_t').pointer())
        unwind_info.add_saved_register('rbp', fp[0])
        unwind_info.add_saved_register('rip', fp[1])

        if frame.is_jitted(fp[0], fp[1]):
            # Our parent frame is jitted.  Again, we are unable to track %rsp
            # properly in the TC, so just preserve its value (just as we do in
            # the TC's custom .eh_frame section).
            unwind_info.add_saved_register('rsp', sp)
        else:
            # Our parent frame is not jitted, so we're in enterTCHelper, and we
            # can restore our parent's %rsp as usual.
            unwind_info.add_saved_register('rsp', fp + 16)

        return unwind_info

register_unwinder(None, HHVMUnwinder())
