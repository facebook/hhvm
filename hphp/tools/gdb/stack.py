"""
GDB commands related to the HHVM stack.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

from compatibility import *

import gdb
from gdbutils import *
import frame


#------------------------------------------------------------------------------
# `walkstk' command.

class WalkstkCommand(gdb.Command):
    """Traverse the interleaved VM and native stacks.

The output backtrace has the following format:

    #<bt frame> <sp> @ <rip>: <function> [at <filename>:<line>]
"""

    def __init__(self):
        super(WalkstkCommand, self).__init__('walkstk', gdb.COMMAND_STACK)

    @errorwrap
    def invoke(self, args, from_tty):
        argv = parse_argv(args)

        if len(argv) > 2:
            print('Usage: walkstk [sp] [rip]')
            return

        # Set sp = $rbp.
        sp_type = T('uintptr_t').pointer()
        sp = gdb.parse_and_eval('$rbp').cast(sp_type)
        if len(argv) >= 1:
            sp = argv[0].cast(sp_type)

        # Set rip = $rip.
        rip_type = T('uintptr_t')
        rip = gdb.parse_and_eval('$rip').cast(rip_type)
        if len(argv) == 2:
            rip = argv[1].cast(rip_type)

        try:
            mcg = V('HPHP::jit::mcg')
            tc_base = mcg['code']['m_base']
            tc_end = tc_base + mcg['code']['m_codeSize']
        except:
            mcg = None

        i = 0
        native_frame = gdb.newest_frame()
        skip_tc = False  # Only used when we can't find HPHP::mcg.

        # Munge `sp' so that it looks like the stack pointer that would point
        # to it if we had another frame---this lets us promote the "increment"
        # to the beginning of the loop, so that we don't miss the final frame.
        sp = (sp, rip)

        while sp:
            rip = sp[1]
            sp = sp[0].cast(sp_type)

            if mcg is not None:
                in_tc = rip >= tc_base and rip < tc_end
            elif not skip_tc:
                # TC frames look like unnamed normal native frames.
                try:
                    next_frame = native_frame.older()
                    in_tc = (next_frame is not None and
                             next_frame.name() is None and
                             next_frame.type() == gdb.NORMAL_FRAME)
                except AttributeError:
                    # No older frame.
                    in_tc = False
            else:
                in_tc = False
                skip_tc = False

            # Try to get the PHP function name from the ActRec at %sp if we're
            # executing in the TC.
            if in_tc:
                ar_type = T('HPHP::ActRec').pointer()
                try:
                    print(frame.stringify(frame.create_php(
                        idx=i + 1, ar=sp.cast(ar_type), rip=rip)))
                except gdb.MemoryError:
                    if mcg is None:
                        # We guessed wrong about whether we're in the TC.
                        skip_tc = True

                    print(frame.stringify(frame.create_native(
                        idx=i + 1, sp=sp, rip=rip)))

            # Pop native frames until we find our current %sp.
            else:
                inlines = 0

                while (native_frame is not None
                       and rip != native_frame.pc()):
                    if inlines > 0 and native_frame.name() is not None:
                        print(frame.stringify(frame.create_native(
                            idx=i,
                            sp='{inline frame}',
                            rip=rip,
                            native_frame=native_frame)))

                    i += 1
                    inlines += 1
                    native_frame = native_frame.older()

                print(frame.stringify(frame.create_native(
                    idx=i, sp=sp, rip=rip, native_frame=native_frame)))

WalkstkCommand()
