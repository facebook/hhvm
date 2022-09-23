# Copyright 2022-present Facebook. All Rights Reserved.

""" Pretty printers for HPHP types """

import lldb
import typing


Formatters = []


def format(datatype: str, regex: bool = True):
    """ Wrapper for pretty printer functions.

    Add the command needed to register the pretty printer with the LLDB debugger
    session once started, to the global Formatters list..

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


@format("HPHP::StringData", regex = False)
def pp_StringData(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    """ Pretty print HPHP::StringData

    Arguments:
        val_obj: an SBValue wrapping an HPHP::StringData
        internal_dict: an LLDB support object not to be used

    Returns:
        A string representing the StringData, or None if there was an error.
    """
    if val_obj.type.IsPointerType():
        return ''

    addr = val_obj.load_addr
    if addr == lldb.LLDB_INVALID_ADDRESS:
        return None
    addr += val_obj.size
    m_len = val_obj.children[1].GetChildMemberWithName("m_len").unsigned
    error = lldb.SBError()
    # +1 for null terminator, it seems
    cstring = val_obj.process.ReadCStringFromMemory(addr, m_len + 1, error)
    if error.Success():
        return cstring
    else:
        print('error: ', error)


@format("HPHP::ArrayData")
def pp_ArrayData(val_obj: lldb.SBValue, _internal_dict) -> typing.Optional[str]:
    """ Pretty print HPHP::ArrayData

    TODO Currently only supports ArrayKind::kVecKind.

    Arguments:
        val_obj: an SBValue wrapping an HPHP::ArrayData
        internal_dict: an LLDB support object not to be used

    Returns:
        A string representing the ArrayData, or None if there was an error.
    """
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


def __lldb_init_module(debugger: lldb.SBDebugger, _internal_dict, top_module=""):
    """ Register the pretty printers in this file with the LLDB debugger.

    Defining this in this module (in addition to the main hhvm module) allows
    this script to be imported into LLDB separately; LLDB looks for a function with
    this name as module load time.

    Arguments:
        debugger: Current debugger object
        _internal_dict: Dict for current script session. For internal use by LLDB only.

    Returns:
        None
    """
    for cmd in Formatters:
        debugger.HandleCommand(cmd(top_module))
