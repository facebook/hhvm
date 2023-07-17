"""
Set the target Unit; used implicitly by some other commands.
"""

import shlex

try:
    # LLDB needs to load this outside of the usual Buck mechanism
    import utils
except ModuleNotFoundError:
    import hhvm_lldb.utils as utils


cur_unit = None


class UnitCommand(utils.Command):
    command = "unit"
    description = "Set the current translation unit"
    epilog = "Use `unit none` to unset. Just `unit` displays the current Unit"

    @classmethod
    def create_parser(cls):
        parser = cls.default_parser()
        parser.add_argument(
            "unit",
            nargs="?",
            help="The translation unit (HPHP::Unit*)",
        )
        return parser

    def __init__(self, debugger, internal_dict):
        super().__init__(debugger, internal_dict)

    def __call__(self, debugger, command, exe_ctx, result):
        global cur_unit

        command_args = shlex.split(command)
        try:
            options = self.parser.parse_args(command_args)
        except SystemExit:
            result.SetError("option parsing failed")
            return

        if options.unit:
            if options.unit.lower() == "none":
                cur_unit = None
            else:
                unit_type = utils.Type("HPHP::Unit", exe_ctx.target).GetPointerType()
                cur_unit = exe_ctx.frame.EvaluateExpression(options.unit).Cast(unit_type)

        if cur_unit:
            result.write(str(cur_unit))
        else:
            result.write("no Unit set")


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
    UnitCommand.register_lldb_command(debugger, __name__, top_module)
