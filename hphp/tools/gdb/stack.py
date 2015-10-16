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

    #<bt frame> <fp> @ <rip>: <function> [at <filename>:<line>]
"""

    def __init__(self):
        super(WalkstkCommand, self).__init__('walkstk', gdb.COMMAND_STACK)

    @errorwrap
    def invoke(self, args, from_tty):
        argv = parse_argv(args)

        if len(argv) > 1:
            print('Usage: walkstk [fp]')
            return

        # Set fp = $rbp.
        fp_type = T('uintptr_t').pointer()
        fp = gdb.parse_and_eval('$rbp').cast(fp_type)
        if len(argv) == 1:
            fp = argv[0].cast(fp_type)[0]

        # Set rip = $rip.
        rip_type = T('uintptr_t')
        rip = gdb.parse_and_eval('$rip').cast(rip_type)
        if len(argv) == 1:
            rip = argv[0].cast(fp_type)[1]

        # Find the starting native frame.
        native_frame = gdb.newest_frame()

        while (native_frame is not None
               and rip != native_frame.pc()):
            native_frame = native_frame.older()

        if native_frame is None:
            if len(argv) == 0:
                print('walkstk: Unknown error: corrupt stack?')
            else:
                print('walkstk: Invalid frame pointer')
            return

        # Get the address and value of `mcg', the global MCGenerator pointer.
        # For some reason, gdb doesn't have debug info about the symbol, so we
        # can't use V(); probably this is because we declare it extern "C" (and
        # maybe also because we do so in a namespace).
        mcg_type = T('HPHP::jit::MCGenerator').pointer()
        mcg_addr = gdb.parse_and_eval('&::mcg').cast(mcg_type.pointer())
        mcg = mcg_addr.dereference()

        # Set the bounds of the TC.
        try:
            tc_base = mcg['code']['m_base']
            tc_end = tc_base + mcg['code']['m_codeSize']
        except:
            # We can't access `mcg' for whatever reason.  Assume that the TC is
            # above the data section, but restricted to low memory.
            tc_base = mcg_addr.cast(T('uintptr_t'))
            tc_end = 0x100000000

        i = 0

        # Make a fake frame for our `fp' and `rip'.  This lets us pop a frame
        # at the top of the loop, which makes it easier to include the final
        # frame.
        fp = (fp, rip)

        while fp:
            rip = fp[1]
            fp = fp[0].cast(fp_type)

            in_tc = rip >= tc_base and rip < tc_end

            # Try to get the PHP function name from the ActRec at %fp if we're
            # executing in the TC.
            if in_tc:
                ar_type = T('HPHP::ActRec').pointer()
                try:
                    print(frame.stringify(frame.create_php(
                        idx=i + 1, ar=fp.cast(ar_type), rip=rip)))
                except gdb.MemoryError:
                    print(frame.stringify(frame.create_native(
                        idx=i + 1, fp=fp, rip=rip, native_frame=native_frame)))

            # Pop native frames until we hit our caller's rip.
            else:
                frames = []

                while (native_frame is not None
                       and fp[1] != native_frame.pc()):
                    frames.append(frame.create_native(
                        idx=i,
                        fp='{inline frame}',
                        rip=rip,
                        native_frame=native_frame))

                    i += 1
                    native_frame = native_frame.older()

                if frames:
                    # Associate the frame pointer with the un-inlined frame.
                    frames[-1]['fp'] = str(fp)

                for f in frames:
                    print(frame.stringify(f))

WalkstkCommand()
