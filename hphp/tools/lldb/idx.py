# Copyright 2022-present Facebook. All Rights Reserved.

import lldb
import shlex
import sys
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

def _unaligned_tv_at_pos_to_tv(base: lldb.SBValue, idx: int) -> lldb.SBValue:
    utv_type = utils.Type("HPHP::UnalignedTypedValue", base.target)
    offset = utv_type.size * idx

    utils.debug_print(f"Loading UnalignedTypedValue (base address: 0x{base.load_addr:x}, offset: {offset})")

    # TODO(michristensen) I believe that creating a child at offset is the preferred way of
    # doing this, but it's not currently working (it's being filled with garbage).
    # utv = base.CreateChildAtOffset('[' + str(idx) + ']', offset, utv_type)

    utv = base.CreateValueFromAddress('[' + str(idx) + ']', base.load_addr + offset, utv_type)

    # Note that we're returning a lldb.SBValue, rather than printing the string,
    # because this is used by a synthetic child provider, and lldb will call the
    # pretty printer associated with it automatically.
    return utv

def _aligned_tv_at_pos_to_tv(base: lldb.SBValue, idx: int) -> lldb.SBValue:
    target = base.target
    quot = idx // 8
    rem = idx % 8
    chunk = base.load_addr + utils.Type("HPHP::PackedBlock", target).size * quot

    val_type = utils.Type("HPHP::Value", target)
    valaddr = chunk + val_type.size * (1 + rem)
    val = base.CreateValueFromAddress("val", valaddr, val_type)

    ty_type = utils.Type("HPHP::DataType", target)
    tyaddr = chunk + rem
    ty = base.CreateValueFromAddress("datatype", tyaddr, ty_type)

    data = val.GetData()
    success = data.Append(ty.data)
    assert success, "Couldn't construct a raw TypedValue"

    tv_type = utils.Type("HPHP::TypedValue", base.target)
    tv = base.CreateValueFromData('[' + str(idx) + ']', data, tv_type)

    return tv

def vec_at(base: lldb.SBValue, idx: int) -> lldb.SBValue:
    # base is a generic pointer to the start of the array of typed values
    try:
        if utils.Global('HPHP::VanillaVec::stores_unaligned_typed_values', base.target):
            return _unaligned_tv_at_pos_to_tv(base, idx)
        else:
            return _aligned_tv_at_pos_to_tv(base, idx)
    except Exception as ex:
        print(f"error while trying to get element #{idx} of vec {str(base)}: {ex}", file=sys.stderr)
        return None

def dict_at(base, idx) -> (str, lldb.SBValue):
    vde_type = utils.Type("HPHP::VanillaDictElm", base.target)
    utils.debug_print(f"Dict base address (i.e. first element): 0x{base.load_addr:x}")
    offset = vde_type.size * idx
    elt = base.CreateValueFromAddress("val", base.load_addr + offset, vde_type)
    utils.debug_print(f"Element #{idx} address: 0x{elt.load_addr:x}")

    try:
        if utils.get(elt, "data", "m_type").signed == utils.Global("HPHP::kInvalidDataType", base.target).signed:
            rawkey = key = '<deleted>'
        elif utils.get(elt, "data", "m_aux", "u_hash").signed < 0:
            ikey = utils.get(elt, "ikey").signed
            rawkey = key = ikey
        else:
            skey = utils.get(elt, "skey")
            rawkey = utils.string_data_val(skey)
            key = f'"{rawkey}"'
    except Exception as e:
        print(f"Failed to get dictionary key with error: {str(e)}", file=sys.stderr)
        rawkey = key = '<invalid>'

    utils._Current_key = rawkey

    try:
        data_raw = utils.get(elt, "data")
        tv_type = utils.Type("HPHP::TypedValue", base.target)
        # Cast because data_raw is a TypedValueAux, and we'd like it to
        # just be presented as a TypedValue.
        data = data_raw.Cast(tv_type)
        # Clone so we can rename it; the name will be used on the
        # left-hand side of the typed value representation, e.g.:
        #   "mykey" = { Double, 3.14 }
        data = data.Clone(str(key))
    except Exception as e:
        print(f"Failed to get dictionary value with error: {str(e)}", file=sys.stderr)
        data = None
    finally:
        utils._Current_key = None

    return data

def keyset_at(base, idx):
    vde_type = utils.Type("HPHP::VanillaKeysetElm", base.target)
    utils.debug_print(f"Keyset base address (i.e. first element): 0x{base.load_addr:x}")
    offset = vde_type.size * idx
    elt = base.CreateValueFromAddress("val", base.load_addr + offset, vde_type)
    utils.debug_print(f"Element #{idx} address: 0x{elt.load_addr:x}")

    try:
        if utils.get(elt, "tv", "m_type").signed == utils.Global("HPHP::kInvalidDataType", base.target).signed:
            key = '<deleted>'
        else:
            key_raw = utils.get(elt, "tv")
            tv_type = utils.Type("HPHP::TypedValue", base.target)
            # Cast because data_raw is a TypedValueAux, and we'd like it to
            # just be presented as a TypedValue
            key = key_raw.Cast(tv_type)
            # Clone so I can rename it (specifically, I don't want anything
            # showing up as the 'index' portion of the listing, so it doesn't
            # look like a vec).
            key = key.Clone("")
    except Exception as e:
        print(f"Failed to get keyset entry with error: {str(e)}", file=sys.stderr)
        key = None

    return key


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
