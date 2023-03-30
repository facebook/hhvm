# Copyright 2022-present Facebook. All Rights Reserved.

""" Pretty printers for HPHP types """

import lldb
import typing

try:
    # LLDB needs to load this outside of the usual Buck mechanism
    import utils
except ModuleNotFoundError:
    import hhvm_lldb.utils as utils


Formatters = []


def format(datatype: str, regex: bool = False):
    """ Wrapper for pretty printer functions.

    Add the command needed to register the pretty printer with the LLDB debugger
    session once started, to the global Formatters list.

    Arguments:
        datatype: the name of the data type being formatted
        regex: whether the datatype in string is a regex

    Returns:
        The original function.
    """
    def inner(func):
        Formatters.append(lambda top_module:
            f'type summary add {"-x" if regex else ""} '
            f'-F {top_module + "." if top_module else ""}pretty.{func.__name__} {datatype}'
        )
        return func
    return inner


#------------------------------------------------------------------------------
# NOTE: the functions prefixed with "pp_" all have the following signature:
#
#   def pp_<TypeName>(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
#      """ Pretty print HPHP::<TypeName>
#          Arguments:
#              val_obj: an SBValue wrapping an HPHP::<TypeName>
#              internal_dict: an LLDB support object not to be used
#
#          Returns:
#              A string representing the <TypeName>, or None if there was an error.
#      """

#------------------------------------------------------------------------------
# TypedValues and its subtypes

@format("^HPHP::(TypedValue|Variant|VarNR)$", regex=True)
def pp_TypedValue(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    if val_obj.type.IsPointerType():
        return ''

    m_type = utils.get(val_obj, "m_type")
    m_data = utils.get(val_obj, "m_data")
    return utils.pretty_tv(m_type, m_data)

#------------------------------------------------------------------------------
# Pointers

def pretty_ptr(val: lldb.SBValue) -> typing.Optional[str]:
    ptr = utils.rawptr(val)
    if utils.is_nullptr(ptr):
        return None

    inner = utils.deref(ptr)
    inner_type = utils.rawtype(inner.type)

    if inner_type.name == "HPHP::StringData":
        return utils.string_data_val(inner)
    return utils.nameof(inner)


@format("^HPHP::req::ptr<.*>$", regex=True)
def pp_ReqPtr(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    return pretty_ptr(val_obj)


@format("^HPHP::(LowPtr<.*>|detail::LowPtrImpl<.*>)$", regex=True)
def pp_LowPtr(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    return pretty_ptr(val_obj)


#------------------------------------------------------------------------------
# Resource

@format("^HPHP::Resource$", regex=True)
def pp_Resource(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    if val_obj.type.IsPointerType():
        return ''
    val = utils.rawptr(utils.get(val_obj, "m_res"))
    return utils.pretty_resource_header(val)


#------------------------------------------------------------------------------
# Strings

@format("^HPHP::StringData$", regex=True)
def pp_StringData(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    if val_obj.type.IsPointerType():
        return ''
    return utils.string_data_val(val_obj)


@format("^HPHP::(Static)?String$", regex=True)
def pp_String(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    if val_obj.type.IsPointerType():
        return ''
    val = utils.rawptr(utils.get(val_obj, "m_str"))
    return utils.string_data_val(val)


@format("^HPHP::StrNR$", regex=True)
def pp_StrNR(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    if val_obj.type.IsPointerType():
        return ''
    val = utils.get(val_obj, "m_px")
    return utils.string_data_val(val)

#------------------------------------------------------------------------------
# Optional

@format("^HPHP(::req)?::Optional<.*>$", regex=True)
def pp_Optional(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    """ See:
          * hphp/runtime/base/req-optional.h
          * hphp/util/optional.h
    """
    val = utils.get(val_obj, "m_opt")
    val = val.children[0] if val.children else None
    return str(val)


#------------------------------------------------------------------------------
# Arrays

def pretty_array_data(val_obj: lldb.SBValue) -> typing.Optional[str]:
    # NOTE: Currently only supports ArrayKind::kVecKind.
    if val_obj.type.IsPointerType():
        return ''

    array_kind = val_obj.target.FindFirstType("HPHP::ArrayData::ArrayKind")
    array_kind_enums = array_kind.GetEnumMembers()
    heap_obj = val_obj.children[0].children[0]  # HPHP::HeapObject
    m_kind = heap_obj.GetChildMemberWithName("m_kind").Cast(array_kind)

    # TODO Try and just compare enums: left is lldb.SBValue, right is lldb.SBTypeEnumMember
    if m_kind.unsigned != array_kind_enums['kVecKind'].unsigned:
        return val_obj
    # Just Vec kind right now

    m_size = val_obj.GetChildMemberWithName("m_size").unsigned
    m_count = heap_obj.GetChildMemberWithName("m_count").unsigned

    # TODO show elements

    return f"ArrayData[{m_kind.name}]: {m_size} element(s) refcount={m_count}"


@format("^HPHP::Array$", regex=True)
def pp_Array(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    if val_obj.type.IsPointerType():
        return ''
    val = utils.rawptr(utils.get(val_obj, "m_arr"))
    return pretty_array_data(val)


@format("HPHP::ArrayData")
def pp_ArrayData(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    return pretty_array_data(val_obj)


#------------------------------------------------------------------------------
# Objects

@format("^HPHP::Object$", regex=True)
def pp_Object(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    val = utils.get(val_obj, "m_obj")
    return utils.nameof(val)


#------------------------------------------------------------------------------
# Extensions

@format("^HPHP::Extension$", regex=True)
def pp_Extension(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    val = utils.deref(val_obj)
    def cstr(v):
        return utils.read_cstring(v, 256, val.process)
    name = cstr(utils.deref(utils.get(val, "m_name")))
    version = cstr(utils.deref(utils.get(val, "m_version")))
    oncall = cstr(utils.deref(utils.get(val, "m_oncall")))
    return f"{name} (version: {version}, oncall: {oncall})"


def __lldb_init_module(debugger: lldb.SBDebugger, _internal_dict, top_module=""):
    """ Register the pretty printers in this file with the LLDB debugger.

    Defining this in this module (in addition to the main hhvm module) allows
    this script to be imported into LLDB separately; LLDB looks for a function with
    this name at module load time.

    Arguments:
        debugger: Current debugger object
        _internal_dict: Dict for current script session. For internal use by LLDB only.

    Returns:
        None
    """
    for cmd in Formatters:
        debugger.HandleCommand(cmd(top_module))
