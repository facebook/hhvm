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
import unwind


#------------------------------------------------------------------------------
# `walkstk' command.

class WalkstkCommand(gdb.Command):
    """Traverse the interleaved VM and native stacks.

The output backtrace has the following format:

    #<bt frame> <fp> @ <rip>: <function> [at <filename>:<line>]

`walkstk' depends on the custom HHVM unwinder defined in unwind.py.
"""

    def __init__(self):
        super(WalkstkCommand, self).__init__('walkstk', gdb.COMMAND_STACK)

    @errorwrap
    def invoke(self, args, from_tty):
        argv = parse_argv(args)

        if len(argv) > 1:
            print('Usage: walkstk [fp]')
            return

        # Bail early if the custom unwinder has not been set up.
        if not unwind.try_unwinder_init():
            print('walkstk: Could not initialize the HHVM unwinder.')

        # Find the starting native frame.
        native_frame = gdb.newest_frame()
        if native_frame is None:
            print('walkstk: Cannot find any frames: corrupt stack?')
            return

        # Set fp = $rbp, rip = $rip.
        fp_type = T('uintptr_t').pointer()
        fp = native_frame.read_register('rbp').cast(fp_type)
        rip = native_frame.pc()

        if len(argv) == 1:
            # Start walking the stack from the user-provided `fp'.
            fp = argv[0].cast(fp_type)[0]
            rip = argv[0].cast(fp_type)[1]

            # Try to find a corresponding native frame.
            while (native_frame is not None
                   and rip != native_frame.pc()):
                native_frame = native_frame.older()

        i = 0

        # Make a fake frame for our `fp' and `rip'.  This lets us pop a frame
        # at the top of the loop, which makes it easier to include the final
        # frame.
        fp = (fp, rip)

        while fp:
            rip = fp[1]
            fp = fp[0].cast(fp_type)

            # Try to get the PHP function name from the ActRec at `fp' if we're
            # executing in the TC.
            if frame.is_jitted(fp, rip):
                ar_type = T('HPHP::ActRec').pointer()
                try:
                    print(frame.stringify(frame.create_php(
                        idx=i, ar=fp.cast(ar_type), rip=rip)))
                except gdb.MemoryError:
                    print(frame.stringify(frame.create_native(
                        idx=i, fp=fp, rip=rip, native_frame=native_frame)))

                if native_frame is not None:
                    native_frame = native_frame.older()
                i += 1

            else:
                if native_frame is None:
                    # If we couldn't find a native frame, then `walkstk' was
                    # invoked with a non-native `fp' argument.  Now that we
                    # don't seem to be in the TC, try to find the corresponding
                    # native frame.
                    native_frame = gdb.newest_frame()
                    while (native_frame is not None
                           and rip != native_frame.pc()):
                        native_frame = native_frame.older()

                if native_frame is not None:
                    # Pop native frames until we hit our caller's rip.
                    frames = []

                    while (native_frame is not None
                           and (fp == 0x0 or fp[1] != native_frame.pc())):
                        frames.append(frame.create_native(
                            idx=i,
                            fp='{inline frame}',
                            rip=native_frame.pc(),
                            native_frame=native_frame))

                        native_frame = native_frame.older()
                        i += 1

                    if frames:
                        # Associate the frame pointer with the un-inlined frame.
                        frames[-1]['fp'] = str(fp)

                    for f in frames:
                        print(frame.stringify(f))
                else:
                    # We only hit this case if gdb undercounted the TC's
                    # frames---which shouldn't happen unless the custom
                    # unwinder (or gdb's unwinder API) is malfunctioning.
                    #
                    # Just guess that the name of the frame is the same as the
                    # name of the block we're in.
                    try:
                        block = gdb.block_for_pc(int(rip))
                        name = block.function.name
                        print(frame.stringify(frame.create_native(
                            idx=i, fp=fp, rip=rip, name='? ' + name)))
                    except:
                        print(frame.stringify(frame.create_native(
                            idx=i, fp=fp, rip=rip, native_frame=None)))
                    i += 1


WalkstkCommand()
