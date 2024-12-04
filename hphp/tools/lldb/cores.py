# pyre-unsafe
"""
LLDB command for extracting various information from cores
"""

import shlex

# pyre-fixme[21]: Could not find module `lldb`.
import lldb

try:
    # LLDB needs to load this outside of the usual Buck mechanism
    # pyre-fixme[21]: Could not find module `utils`.
    import utils
except ModuleNotFoundError:
    import hphp.tools.lldb.utils as utils


# ------------------------------------------------------------------------------
# `extract` command.


class ExtractCommand(utils.Command):
    command = "extract"
    description = (
        "Extract information like stacktrace, JIT profile, etc. from the core file"
    )

    @classmethod
    def create_parser(cls):
        parser = cls.default_parser()
        parser.add_argument(
            "type",
            choices=["jitprof", "perfmap", "stacktrace"],
            help="Type of data to extract",
        )
        parser.add_argument(
            "--outfile",
            help="The file to store the extracted data in (if not specified, displayed in LLDB)",
        )
        return parser

    def __init__(self, debugger, internal_dict):
        super().__init__(debugger, internal_dict)

    def __call__(
        self,
        # pyre-fixme[11]: Annotation `SBDebugger` is not defined as a type.
        debugger: lldb.SBDebugger,
        command: str,
        # pyre-fixme[11]: Annotation `SBExecutionContext` is not defined as a type.
        exe_ctx: lldb.SBExecutionContext,
        # pyre-fixme[11]: Annotation `SBCommandReturnObject` is not defined as a type.
        result: lldb.SBCommandReturnObject,
    ):
        command_args = shlex.split(command)
        try:
            options = self.parser.parse_args(command_args)
        except SystemExit:
            result.SetError("option parsing failed")
            return

        start_var = f"s_{options.type}_start"
        start = exe_ctx.target.FindFirstGlobalVariable(start_var)
        if start.GetError().Fail():
            result.SetError(
                f"Unable to access start variable '{start_var}': {start.GetError()}"
            )
            return
        start_addr = int(start.GetValue(), 0)

        end_var = f"s_{options.type}_end"
        end = exe_ctx.target.FindFirstGlobalVariable(end_var)
        if end.GetError().Fail():
            result.SetError(
                f"Unable to access end variable '{end_var}': {end.GetError()}"
            )
            return
        end_addr = int(end.GetValue(), 0)

        table_size = end_addr - start_addr
        result.write(f"Found table at {start_addr} with size {table_size}\n")

        err = lldb.SBError()
        memory = exe_ctx.target.ReadMemory(
            lldb.SBAddress(start_addr, exe_ctx.target), table_size, err
        )
        if err.Fail():
            result.SetError(f"Error reading memory: {err}")
            return

        if options.outfile:
            with open(options.outfile, "wb") as f:
                f.write(memory)
                result.write(f"Wrote to {options.outfile}\n")
        else:
            result.write(memory.decode("utf-8"))


def __lldb_init_module(debugger, _internal_dict, top_module=""):
    """Register the commands in this file with the LLDB debugger.

    Defining this in this module (in addition to the main hhvm module) allows
    this script to be imported into LLDB separately; LLDB looks for a function with
    this name at module load time.

    Arguments:
        debugger: Current debugger object
        _internal_dict: Dict for current script session. For internal use by LLDB only.

    Returns:
        None
    """
    ExtractCommand.register_lldb_command(debugger, __name__, top_module)
