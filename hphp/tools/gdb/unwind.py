#!/usr/bin/env python3

"""
GDB commands related to the HHVM stack.
"""

from compatibility import *

import gdb
from gdb.unwinder import Unwinder, register_unwinder
from gdbutils import *
import frame


#------------------------------------------------------------------------------
# HHVM unwinder.

class FrameId:
    __slots__ = ['sp', 'pc', 'special']

    def __init__(self, sp, ip, special):
        self.sp = sp
        self.pc = ip
        self.special = special


class HHVMUnwinder(Unwinder):
    """Custom unwinder for jitted frames.

Chases the frame pointer chain in frames which appear to be in the TC, and
falls back to the default GDB unwinder(s) otherwise.
"""

    def __init__(self):
        self.regs = arch_regs()
        super(HHVMUnwinder, self).__init__('hhvm_unwinder')

    def __call__(self, pending_frame):
        fp = pending_frame.read_register(self.regs['fp'])
        sp = pending_frame.read_register(self.regs['sp'])
        ip = pending_frame.read_register(self.regs['ip'])

        if not frame.is_jitted(fp, ip):
            return None

        # GDB wants a FrameId to be a pair of (stack pointer, starting PC) for
        # a given function frame.  Unfortunately, we can't restore the stack
        # pointer correctly in the TC, and we don't know our starting IP.
        #
        # Instead, we just use the stack pointer value for our most
        # recent call into native code, along with the current PC.  It
        # turns out that this is almost good enough. The problem is
        # that we can end up with two frames with the same sp and pc
        # in the same vm nesting (especially if we have interp ret
        # helpers as the return address), and if two frame ids are the
        # same, gdb bails out with an error.
        #
        # GDB expects stack pointers to be monotonically nondecreasing
        # as we unwind, so we can't use fp instead of sp, but FrameIds
        # are allowed to have a "special" field thats used to
        # differentiate otherwise identical ones; so use fp there
        # instead.
        unwind_info = pending_frame.create_unwind_info(FrameId(sp, ip, fp))

        # Restore the saved frame pointer and instruction pointer.
        fp = fp.cast(T('uintptr_t').pointer())
        unwind_info.add_saved_register(self.regs['fp'], fp[0])
        unwind_info.add_saved_register(self.regs['ip'], fp[1])

        if frame.is_jitted(fp[0], fp[1]):
            # Our parent frame is jitted.  Again, we are unable to track %rsp
            # properly in the TC, so just preserve its value (just as we do in
            # the TC's custom .eh_frame section).
            unwind_info.add_saved_register(self.regs['sp'], sp)
        else:
            # Our parent frame is not jitted, so we're in g_ustubs.enterTCHelper,
            # which has a special restore sequence
            i = 0
            for r in self.regs['cross_jit_save']:
                i -= 1
                unwind_info.add_saved_register(r, fp[i])
            unwind_info.add_saved_register(self.regs['sp'], fp + 2)

        return unwind_info


#------------------------------------------------------------------------------
# Unwinder initialization.

_did_init = False


def try_unwinder_init():
    """Try to register the custom unwinder if it hasn't been already, and
    return whether the unwinder has been successfully registered."""
    global _did_init

    if _did_init:
        return True

    # If we can't successfully call is_jitted(), then gdb startup hasn't
    # proceeded to a point where it's safe to set up our unwinder yet, so just
    # bail out.
    try:
        frame.is_jitted(0, 0)
    except:
        return False

    register_unwinder(None, HHVMUnwinder())
    frame.JittedFrameFilter()
    _did_init = True

    return True


class UnwinderCommand(gdb.Command):
    """Manage the custom unwinder."""

    def __init__(self):
        super(UnwinderCommand, self).__init__('unwinder', gdb.COMMAND_STACK,
                                              gdb.COMPLETE_NONE, True)


UnwinderCommand()


class UnwinderInitCommand(gdb.Command):
    """Initialize the custom unwinder."""

    def __init__(self):
        super(UnwinderInitCommand, self).__init__('unwinder init',
                                                  gdb.COMMAND_STACK)

    @errorwrap
    def invoke(self, args, from_tty):
        if try_unwinder_init():
            print('HHVM unwinder has been initialized.')
        else:
            print('HHVM unwinder could not be initialized.')
            print('Has gdb startup run to completion?')


UnwinderInitCommand()


def handle_before_prompt():
    global _did_init
    if _did_init:
        return
    if try_unwinder_init():
        gdb.events.before_prompt.disconnect(handle_before_prompt)


gdb.events.before_prompt.connect(handle_before_prompt)
