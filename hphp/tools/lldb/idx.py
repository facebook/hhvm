# Copyright 2022-present Facebook. All Rights Reserved.

import lldb
import shlex
import utils


def fixed_vector_at(fv: lldb.SBValue, idx: int, hasher=None):
    ptr = utils.rawptr(fv.GetChildMemberWithName("m_impl").GetChildMemberWithName("m_sp"))
    return ptr.GetChildAtIndex(idx, lldb.eDynamicDontRunTarget, True)


@utils.memoized
def idx_accessors():
    return {
        'HPHP::FixedVector': fixed_vector_at,
    }


def idx(container: lldb.SBValue, index, hasher=None):
    if container.type.IsPointerType():
        container = container.deref

    container_type = utils.template_type(container.type)
    true_type = utils.template_type(container.type.GetCanonicalType())

    accessors = idx_accessors()

    if container_type in accessors:
        value = accessors[container_type](container, index, hasher)
    elif true_type in accessors:
        value = accessors[true_type](container, index, hasher)
    else:
        print(f'idx: Unrecognized container ({container_type} - {true_type}).')
        return None

    return value


class IdxCommand(utils.Command):
    command = "idx"
    description = "Index into an arbitrary container"
    epilog = """\
LLDB `print` is called on the address of the value, and then the value itself is
printed.

If `container' is of a recognized type (e.g., native arrays, std::vector),
`idx' will index according to operator[]. Otherwise, it will attempt to treat
`container' as an object with data member `key'.

If `container' is accessed by hashing `key', an optional `hasher' specification
(a bare word string, such as "id", sans quotes) may be passed. The specified
hash, if valid, will be used instead of the default hash for the key type.
"""

    @classmethod
    def create_parser(cls):
        parser = cls.default_parser()
        parser.add_argument(
            "container",
            help="A container to index into"
        )
        parser.add_argument(
            "key",
            help="The index or data member to access"
        )
        parser.add_argument(
            "hasher",
            default=id,
            nargs="?",
            help="An optional hasher specification (a bare word string, such as \"id\" sans quotes)"
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
        if options.hasher is not id:
            # TODO handle hasher option
            result.SetError("invalid number of arguments")
            return
        try:
            # TODO Handle non-int keys
            index = int(options.key)
        except ValueError:
            result.SetError("invalid key (only int currently supported)")
            return

        res = idx(container, index)
        if res is None:
            result.SetError(f"cannot access {index} in {container}")
            return

        result.write(str(res))


def __lldb_init_module(debugger, _internal_dict, top_module=""):
    """ Register the commands in this file with the LLDB debugger.

    Defining this in this module (in addition to the main hhvm module) allows
    this script to be imported into LLDB separately; LLDB looks for a function with
    this name as module load time.

    Arguments:
        debugger: Current debugger object
        _internal_dict: Dict for current script session. For internal use by LLDB only.

    Returns:
        None
    """
    IdxCommand.register_lldb_command(debugger, __name__, top_module)
