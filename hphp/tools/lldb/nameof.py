# pyre-strict
"""
LLDB command for printing the names of various objects
"""

import argparse
import shlex

import lldb

try:
    # LLDB needs to load this outside of the usual Buck mechanism
    # pyre-fixme[21]: Could not find module `utils`.
    import utils
except ModuleNotFoundError:
    import hphp.tools.lldb.utils as utils


# ------------------------------------------------------------------------------
# `nameof` command.


class NameOfCommand(utils.Command):
    command = "nameof"
    description = (
        "Print the name of various HHVM objects, like functions, classes, or objects"
    )

    @classmethod
    def create_parser(cls) -> argparse.ArgumentParser:
        parser = cls.default_parser()
        parser.add_argument("object", help="An HHVM object to get the name of")
        return parser

    def __call__(
        self,
        debugger: lldb.SBDebugger,
        command: str,
        exe_ctx: lldb.SBExecutionContext,
        result: lldb.SBCommandReturnObject,
    ) -> None:
        command_args = shlex.split(command)
        try:
            options = self.parser.parse_args(command_args)
        except SystemExit:
            result.SetError("option parsing failed")
            return

        object = exe_ctx.frame.EvaluateExpression(options.object)
        name = utils.nameof(object)

        if name is not None:
            result.write(name)
        else:
            result.SetError("unrecognized object")


def __lldb_init_module(
    debugger: lldb.SBDebugger,
    top_module: str = "",
) -> None:
    """Register the commands in this file with the LLDB debugger.

    Defining this in this module (in addition to the main hhvm module) allows
    this script to be imported into LLDB separately; LLDB looks for a function with
    this name at module load time.

    Arguments:
        debugger: Current debugger object

    Returns:
        None
    """
    NameOfCommand.register_lldb_command(debugger, __name__, top_module)
