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

`walkstk' relies on gdb's internal representation of HHVM's jitted TC frames
being in one-to-one correspondence with frame records discovered via a simple
walk of the %fp chain.  (The metadata kept for these frames, however, needn't
be correct or coherent.)

The heuristics that gdb uses to unwind the stack vary by version, and if gdb
does not meet the above expectations, `walkstk' may behave aberrantly.  If gdb
sees too few frames, `walkstk' may drop some native frame metadata (such as
function names).  If `gdb' sees too many frames, `walkstk' may display garbage
frames.
"""

    def __init__(self):
        super(WalkstkCommand, self).__init__('walkstk', gdb.COMMAND_STACK)

    @errorwrap
    def invoke(self, args, from_tty):
        argv = parse_argv(args)

        if len(argv) > 1:
            print('Usage: walkstk [fp]')
            return

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

        # Get the value of `mcg', the global MCGenerator pointer.
        mcg = V('HPHP::jit::mcg')

        # Set the bounds of the TC.
        try:
            tc_base = mcg['code']['m_base']
            tc_end = tc_base + mcg['code']['m_codeSize']
        except:
            # We can't access `mcg' for whatever reason---maybe it's gotten
            # corrupted somehow.  Assume that the TC is above the data section,
            # but restricted to low memory.
            tc_base = mcg.address.cast(T('uintptr_t'))
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

            # Try to get the PHP function name from the ActRec at `fp' if we're
            # executing in the TC.
            if in_tc:
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
                           and fp[1] != native_frame.pc()):
                        frames.append(frame.create_native(
                            idx=i,
                            fp='{inline frame}',
                            rip=rip,
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
                    # frames.  Guess that the name of the frame is the same as
                    # the name of the block we're in.
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
