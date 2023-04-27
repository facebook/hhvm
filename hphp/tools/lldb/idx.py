# Copyright 2022-present Facebook. All Rights Reserved.

import lldb
import shlex
import typing

try:
    # LLDB needs to load this outside of the usual Buck mechanism
    import utils
except ModuleNotFoundError:
    import hhvm_lldb.utils as utils


def at(ptr: lldb.SBValue, idx: int) -> typing.Optional[lldb.SBValue]:
    """ Access ptr[idx] """
    val = ptr.GetChildAtIndex(idx, lldb.eDynamicDontRunTarget, True)
    if not val.IsValid():
        return None
    return val


def atomic_low_ptr_vector_at(av: lldb.SBValue, idx: int, hasher=None) -> typing.Optional[lldb.SBValue]:
    """ Get the value at idx in the atomic vector av

    See hphp/util/atomic-vector.h

    Arguments:
        av: The vector, represented as lldb.SBValue[HPHP::AtomicLowPtrVector]
        idx: Index to get
        hasher: (Not yet implemented)

    Returns:
        av[ix] if valid, otherwise None
    """

    if hasher:
        # TODO implement
        raise NotImplementedError("hasher argument currently unused")

    size = utils.get(av, "m_size").unsigned

    if idx < size:
        unique_ptr = utils.rawptr(utils.get(av, "m_vals"))
        return at(unique_ptr, idx)
    else:
        return atomic_low_ptr_vector_at(utils.atomic_get(utils.get(av, 'm_next')), idx - size)


def fixed_vector_at(fv: lldb.SBValue, idx: int, hasher=None) -> typing.Optional[lldb.SBValue]:
    """ Get the value at idx in the fixed vector fv

    See hphp/util/fixed-vector.h

    Arguments:
        fv: The vector, represented as lldb.SBValue[HPHP::FixedVector]
        idx: Index to get
        hasher: (Not yet implemented)

    Returns:
        fv[ix] if valid, otherwise None
    """
    if hasher is not None:
        # TODO implement
        raise NotImplementedError("hasher argument currently unused")

    ptr = utils.rawptr(utils.get(fv, "m_impl", "m_sp"))
    return at(ptr, idx)


def compact_vector_at(cv: lldb.SBValue, idx: int, hasher=None) -> typing.Optional[lldb.SBValue]:
    """ Get the value at idx in the compact vector cv

    Arguments:
        cv: The vector, repesented as lldb.SBValue[HPHP::CompactVector]
        idx: Index to get
        hasher: (Not yet implemented)

    Returns:
        cv[ix] if in bounds, otherwise None

    See hphp/util/compact-vector.h
    """

    if hasher is not None:
        # TODO implement
        raise NotImplementedError("hasher argument currently unused")

    if utils.get(cv, "m_data").unsigned == 0:  # null ptr
        return None

    sz = utils.get(cv, "m_data", "m_len").unsigned
    if idx >= sz:
        return None

    inner = cv.type.template_argument(0)
    elems = (utils.get(cv, "m_data").Cast(utils.Type('char').GetPointerType())
             + utils.get(cv, "elems_offset").Cast(inner.GetPointerType()))
    return at(elems, idx)


@utils.memoized
def idx_accessors():
    return {
        "HPHP::AtomicLowPtrVector": atomic_low_ptr_vector_at,
        "HPHP::CompactVector": compact_vector_at,
        "HPHP::FixedVector": fixed_vector_at,
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
    this name at module load time.

    Arguments:
        debugger: Current debugger object
        _internal_dict: Dict for current script session. For internal use by LLDB only.

    Returns:
        None
    """
    IdxCommand.register_lldb_command(debugger, __name__, top_module)
