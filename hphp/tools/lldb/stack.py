# Copyright 2022-present Facebook. All Rights Reserved

import lldb
import shlex
import typing

try:
    # LLDB needs to load this outside of the usual Buck mechanism
    import frame
    import idx
    import utils
except ModuleNotFoundError:
    import hhvm_lldb.frame as frame
    import hhvm_lldb.idx as idx
    import hhvm_lldb.utils as utils

#------------------------------------------------------------------------------
# Helpers.

def function_name_for_rip(rip: typing.Union[int, lldb.SBValue], target: lldb.SBTarget) -> typing.Optional[str]:
    """ Try getting the name of the function containing `rip`.

    Arguments:
        rip: the instruction pointer (i.e. PC), either an an int or SBValue
        target: the debugging session target

    Returns:
        Name of the function container `rip`, or None if `rip` does not belong
        to a native function
    """
    addr = rip.unsigned if isinstance(rip, lldb.SBValue) else rip
    try:
        addr = lldb.SBAddress(addr, target)
        return addr.function.name
    except Exception:
        return None


class WalkstkCommand(utils.Command):
    command = "walkstk"
    description = "Traverse the interleaved VM and native stacks"
    epilog = """\
The output backtrace has the following format:

    #<bt frame> <fp> @ <rip>: <function> [at <filename>:<line>]

`walkstk` depends on the custom HHVM unwinder defined in unwind.py.
"""

    @classmethod
    def create_parser(cls):
        parser = cls.default_parser()
        parser.add_argument(
            "fp",
            type=int,
            nargs="?",
            help="Optional address to use as the starting frame pointer. "
                 "If not given, walkstk will use the current thread's FP.",
        )
        return parser

    def __init__(self, debugger, internal_dict):
        super().__init__(debugger, internal_dict)

    def __call__(self, debugger, command, exe_ctx, result):
        command_args = shlex.split(command)
        try:
            options = self.parser.parse_args(command_args)
        except Exception:
            result.SetError("option parsing failed")
            return

        # TODO check for custom unwinder being set up

        # Find the starting native frame
        native_frame = exe_ctx.frame
        if not native_frame.IsValid():
            result.SetError('walkstk: Cannot find any frames: corrupt stack?')
            return

        fp_type = utils.Type("uintptr_t", exe_ctx.target).GetPointerType()
        # Note: You can use frame.fp to get an unsigned int; I'm using frame.registers[name]
        #       because I want an SBValue instead, for easier casting and consistency
        fp = utils.reg("fp", native_frame).Cast(fp_type)
        rip = utils.reg("ip", native_frame).Cast(fp_type)

        if options.fp:
            # Start walking the stack from the user-provided `fp`
            fp = exe_ctx.frame.EvaluateExpression(options.fp).Cast(fp_type)
            rip = idx.at(fp, 1)

            # Try to find a corresponding native frame
            while (native_frame is not None
                    and rip.unsigned != native_frame.pc):
                native_frame = native_frame.parent

        i = 0

        # Make a fake frame for our `fp` and `rip`.  This lets us pop a frame
        # at the top of the loop, which makes it easier to include the final
        # frame.
        fp = (fp, rip)

        while True:
            # In the frame pointed to by `fp`, the previous `$rbp` value is found
            # at fp[0], and the saved `$rip` value is found 8 bytes
            # after that, i.e. fp[1] (i.e. fp[0] + sizeof(uintptr_t)).
            # Note that this also corresponds to the first two uint64_t fields
            # of HPHP::ActRec.
            if isinstance(fp, tuple):
                rip = fp[1]
                fp = fp[0]
            else:
                rip = idx.at(fp, 1).Cast(fp_type)
                fp = idx.at(fp, 0).Cast(fp_type)

            # Try to get the PHP function name from the ActRec at `fp` if we're
            # executing in the TC.
            if frame.is_jitted(rip):
                ar_type = utils.Type("HPHP::ActRec", exe_ctx.target).GetPointerType()
                try:
                    result.write(frame.stringify(frame.create_php(idx=i, ar=fp.Cast(ar_type), rip=rip)) + "\n")
                except Exception:
                    result.write(frame.stringify(frame.create_native(idx=i, fp=fp, rip=rip, native_frame=native_frame)) + "\n")

                if native_frame is not None:
                    native_frame = native_frame.parent
                i += 1
            else:
                if native_frame is None:
                    # If we couldn't find a native frame, then `walkstk` was
                    # invoked with a non-native `fp` argument.  Now that we
                    # don't seem to be in the TC, try to find the corresponding
                    # native frame.
                    native_frame = exe_ctx.frame
                    while (native_frame is not None
                           and rip.unsigned != native_frame.pc):
                        native_frame = native_frame.parent

                if native_frame is not None:
                    # Pop native frames until we hit our caller's rip.
                    frames = []

                    saved_rip = idx.at(fp, 1).unsigned
                    while (native_frame.IsValid() and (fp.unsigned == 0x0 or saved_rip != native_frame.pc)):
                        frames.append(frame.create_native(
                            idx=i,
                            fp='{inline frame}',
                            rip=native_frame.pc,
                            native_frame=native_frame))

                        native_frame = native_frame.parent
                        i += 1

                    if frames:
                        # Associate the frame pointer with the un-inlined frame.
                        frames[-1].fp = frame.format_ptr(fp)

                    for f in frames:
                        result.write(frame.stringify(f) + "\n")
                else:
                    # We only hit this case if lldb undercounted the TC's
                    # frames, which shouldn't happen unless the custom
                    # unwinder (or lldb's unwinder API) is malfunctioning.
                    #
                    # Just guess that the name of the frame is the same as the
                    # name of the block we're in.
                    try:
                        result.write(frame.stringify(frame.create_native(
                            idx=i, fp=fp, rip=rip, name=function_name_for_rip(rip, exe_ctx.target))) + "\n")
                    except Exception:
                        result.write(frame.stringify(frame.create_native(
                            idx=i, fp=fp, rip=rip, native_frame=None)) + "\n")
                    i += 1

            if fp.unsigned == 0:
                break


def __lldb_init_module(debugger, _internal_dict, top_module=""):
    """ Register the stack-related commands in this file with the LLDB debugger.

    Defining this in this module (in addition to the main hhvm module) allows
    this script to be imported into LLDB separately; LLDB looks for a function with
    this name at module load time.

    Arguments:
        debugger: Current debugger object
        _internal_dict: Dict for current script session. For internal use by LLDB only.

    Returns:
        None
    """
    WalkstkCommand.register_lldb_command(debugger, __name__, top_module)
