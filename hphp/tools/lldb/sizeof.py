"""
LLDB command for printing the sizes of various containers.
"""

import lldb
import shlex

try:
    # LLDB needs to load this outside of the usual Buck mechanism
    import utils
except ModuleNotFoundError:
    import hhvm_lldb.utils as utils

#------------------------------------------------------------------------------
# Size accessor.

def sizeof(container: lldb.SBValue) -> lldb.SBValue:
    """ Get the actual semantic size (i.e. number of elements) of a container

    Arguments:
        container: the container to get the size of
    Returns:
        An SBValue corresponding the field where the size information is stored.
    """
    container = utils.deref(container)
    t = utils.template_type(utils.rawtype(container.type))

    if t == "std::vector" or t == "HPHP::req::vector":
        impl = utils.get(container, "_M_impl")
        return utils.get(impl, "_M_finish") - utils.get(impl, "_M_start")
    elif t == "std::priority_queue":
        return sizeof(utils.get(container, "c"))
    elif t == 'std::unordered_map' or t == 'HPHP::hphp_hash_map':
        return utils.get(container, "_M_h", "_M_element_count")
    elif t == 'HPHP::FixedStringMap':
        return utils.get(container, "m_extra")
    elif t == 'HPHP::IndexedStringMap':
        return utils.get(container, "m_map", "m_extra")


#------------------------------------------------------------------------------
# `sizeof` command.

class SizeOfCommand(utils.Command):
    command = "sizeof"
    description = "Print the semantic size a container"

    @classmethod
    def create_parser(cls):
        parser = cls.default_parser()
        parser.add_argument(
            "container",
            help="A container to get the size of"
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

        container = exe_ctx.frame.EvaluateExpression(options.container)
        size = sizeof(container)

        if size is not None:
            result.write(str(size.unsigned))
        else:
            result.SetError("unrecognized container")


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
    SizeOfCommand.register_lldb_command(debugger, __name__, top_module)
