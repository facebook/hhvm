"""
GDB commands related to the HHVM stack.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

from compatibility import *

import gdb
import re
from gdbutils import *
import frame
import unwind


#------------------------------------------------------------------------------
# Helpers.

def _function_for(rip):
    """Get the name of the function containing `rip', or None if `rip' does not
    belong to a native function."""
    try:
        out = gdb.execute('x/i %s' % str(rip), False, True)
        # [=>] 0xabc3210 <HPHP::foo<T>((*) Foo*(int))+789>: jmp 0xf0 <...>
        return re.split('\+\d+>:', out.split('<', 1)[1], 1)[0]
    except:
        return None

    # We really ought to be able to do this using the gdb.Block API, but in
    # practice it sometimes produces an entirely unrelated block---maybe due to
    # debuginfo corruption from LTO, or template folding, or something else
    # entirely.  The code should look something like:
    #
    # block = gdb.block_for_pc(int(rip))
    #
    # while block is not None:
    #     if block.function is not None:
    #         return block.function.name
    #
    #     block = block.superblock
    #
    # return None


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
                        print(frame.stringify(frame.create_native(
                            idx=i, fp=fp, rip=rip, name=_function_for(rip))))
                    except:
                        print(frame.stringify(frame.create_native(
                            idx=i, fp=fp, rip=rip, native_frame=None)))
                    i += 1


WalkstkCommand()


#------------------------------------------------------------------------------
# `walkfp' command.

class WalkfpCommand(gdb.Command):
    """Traverse the interleaved VM and native stacks.

This is a simplified version of `walkstk' which carries fewer dependencies.
Its output is not as detailed, but it is more robust to failures (including
internal gdb errors).

The output backtrace has the following format:

    #<idx> <fp> @ <rip>: <function> [at <filename>:<line>]
"""

    def __init__(self):
        super(WalkfpCommand, self).__init__('walkfp', gdb.COMMAND_STACK)

    @errorwrap
    def invoke(self, args, from_tty):
        argv = parse_argv(args)

        if len(argv) > 2:
            print('Usage: walkfp [fp] [rip]')
            return

        fp_type = T('uintptr_t').pointer()
        fp = gdb.parse_and_eval('$rbp').cast(fp_type)
        rip = gdb.parse_and_eval('$rip').cast(T('uintptr_t'))

        if len(argv) >= 1:
            fp = argv[0].cast(fp_type)

            if len(argv) == 2:
                rip = argv[1].cast(T('uintptr_t'))

        i = 0
        fp = (fp, rip)

        while fp:
            rip = fp[1]
            fp = fp[0].cast(fp_type)

            try:
                if frame.is_jitted(fp, rip):
                    ar_type = T('HPHP::ActRec').pointer()
                    print(frame.stringify(frame.create_php(
                        idx=i, ar=fp.cast(ar_type), rip=rip)))
                else:
                    print(frame.stringify(frame.create_native(
                        idx=i, fp=fp, rip=rip, name=_function_for(rip))))
            except:
                print(frame.stringify(frame.create_native(idx=i, fp=fp, rip=rip)))

            i += 1


WalkfpCommand()
