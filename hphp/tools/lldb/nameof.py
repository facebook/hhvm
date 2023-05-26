"""
LLDB command for printing the names of various objects
"""

import shlex

try:
    # LLDB needs to load this outside of the usual Buck mechanism
    import utils
except ModuleNotFoundError:
    import hhvm_lldb.utils as utils


#------------------------------------------------------------------------------
# `sizeof` command.

class NameOfCommand(utils.Command):
    command = "nameof"
    description = "Print the name of various HHVM objects, like functions, classes, or objects"

    @classmethod
    def create_parser(cls):
        parser = cls.default_parser()
        parser.add_argument(
            "object",
            help="An HHVM object to get the name of"
        )
        return parser

    def __init__(self, debugger, internal_dict):
        super().__init__(debugger, internal_dict)

    def __call__(self, debugger, command, exe_ctx, result):
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


def __lldb_init_module(debugger, _internal_dict, top_module=""):
    """ Register the commands in this file with the LLDB debugger.

    Defining this in this module (in addition to the main hhvm module) allows
    this script to be imported into LLDB separately; LLDB looks for a function with
    this name at module load time.

    Arguments:
        debugger: Current debugger object
        _internal_dict: Dict for current script session. For internal use by LLDB only.

    Returns:
        None
    """
    NameOfCommand.register_lldb_command(debugger, __name__, top_module)
