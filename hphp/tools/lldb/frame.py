# Copyright 2022-present Facebook. All Rights Reserved

import lldb
import dataclasses
import typing

try:
    # LLDB needs to load this outside of the usual Buck mechanism
    import idx
    import lookup
    import utils
except ModuleNotFoundError:
    import hhvm_lldb.idx as idx
    import hhvm_lldb.lookup as lookup
    import hhvm_lldb.utils as utils


# Making a dataclass rather than namedtuple because we alter it during walkstk
@dataclasses.dataclass
class Frame:
    idx: int
    fp: str
    rip: str
    func: str
    file: typing.Optional[str] = None
    line: typing.Optional[str] = None


#------------------------------------------------------------------------------
# Frame sniffing.

def is_jitted(ip: lldb.SBValue) -> bool:
    """ Determine if the instruction pointer points inside the region of jitted code

    Arguments:
        ip: The instruction pointer

    Returns:
        Whether `ip` refers to jitted code
    """
    # Get the value of the global CodeCache pointer.
    g_code = utils.Global("HPHP::jit::tc::g_code", ip.target)
    code_size = utils.get(g_code, "m_codeSize")

    # Set the bounds of the TC.
    tc_base = utils.get(g_code, "m_base")
    tc_end = lldb.SBAddress(tc_base.unsigned, tc_base.target)
    tc_end.OffsetAddress(code_size.unsigned)

    if not tc_base.IsValid() or not tc_end.IsValid():
        # We can't access `g_code' for whatever reason---maybe it's gotten
        # corrupted somehow.  Assume that the TC is above the data section,
        # but restricted to low memory.
        tc_base = g_code.addr  # vs. g_code.unsigned
        tc_end = 0x100000000
    else:
        tc_base = tc_base.unsigned
        tc_end = tc_end.GetLoadAddress(ip.target)

    return ip.unsigned >= tc_base and ip.unsigned < tc_end


#------------------------------------------------------------------------------
# Frame builders.


def create_native(
    idx: int,
    fp: typing.Union[str, lldb.SBValue],
    rip: lldb.SBValue,
    native_frame: typing.Optional[lldb.SBFrame]=None,
    name: typing.Optional[str]=None,
) -> Frame:
    """ Collect metadata for a native frame.

    Args:
        idx: This frame's location in the stack
        fp: The frame pointer
        rip: The instruction pointer
        native_frame: The lldb.SBFrame object
        name: Optional name of the current function

    Returns:
        A Frame object
    """
    # Try to get the function name.
    if native_frame is None:
        if name is None:
            func_name = "<unknown>"
        else:
            func_name = name
    else:
        fn = native_frame.GetFunctionName()  # When not None, returns function name + template args + parameters
        if fn is None:
            func_name = "<unknown>"
        else:
            end = fn.rfind("(")
            if end == -1:
                func_name = fn
            else:
                func_name = fn[:fn.rfind("(")] + "()"

    frame = Frame(
        idx=idx,
        fp=format_ptr(fp),
        rip=format_ptr(rip),
        func=func_name,
    )

    if native_frame is None:
        return frame

    line_entry = native_frame.line_entry

    # Munge and print the code location if we have one.
    if line_entry.IsValid():
        frame.file = line_entry.file.fullpath  # TODO decide if we want this whole path
        frame.line = line_entry.line

    return frame


def create_php(
    idx: int,
    ar: lldb.SBValue,
    rip: typing.Union[str, lldb.SBValue]='0x????????',
    pc: lldb.SBValue=None,
) -> Frame:
    """Collect metadata for a PHP frame.

    Args:
        idx: Index of the current frame
        ar: The activation record (lldb.SBValue[HPHP::ActRec])
        rip: The instruction pointer

    Returns:
        A Frame object
    """
    func = lookup.lookup_func_from_frame_pointer(ar)  # lldb.SBValue[HPHP::Func *]
    shared = utils.rawptr(utils.get(func, "m_shared"))  # lldb.SBValue[HPHP::SharedData]
    flags = utils.get(shared, "m_allFlags")  # lldb.SBValue[HPHP::Func::SharedData::Flags]

    shared_type = utils.rawtype(shared.type).GetPointeeType()
    assert shared_type.name == "HPHP::Func::SharedData", f"create_php: Expected m_shared to point to HPHP::Func::SharedData, it points to {shared_type.name}"

    # Pull the function name.
    if utils.get(flags, "m_isClosureBody").unsigned == 0:
        func_name = utils.nameof(func)
    else:
        # TODO test this code path
        class_type = utils.Type("HPHP::Class", ar.target).GetPointerType()
        func_name = utils.nameof(utils.get(func, "m_baseCls").Cast(class_type))
        func_name = func_name.split(";")[0]

    if len(func_name) == 0:
        func_name = "<pseudomain>"

    frame = Frame(
        idx=idx,
        fp=format_ptr(ar),
        rip=format_ptr(rip),
        func=f"[PHP] {func_name}()"
    )

    attrs = utils.atomic_get(utils.get(func, "m_attrs", "m_attrs"))  # HPHP::Attr
    attr_builtin = utils.Enum("HPHP::Attr", "AttrBuiltin", ar.target)
    if attrs.unsigned & attr_builtin.unsigned:
        # Builtins don't have source files.
        return frame

    # Pull the PC from Func::base() and ar->m_callOff if necessary.
    if pc is None:
        m_bc = utils.get(shared, "m_bc")
        bc = utils.rawptr(m_bc)
        pc = bc.unsigned + (utils.get(ar, "m_callOffAndFlags").unsigned >> 2)

    frame.file = php_filename(func, ar.target)
    frame.line = php_line_number(func, pc)

    return frame


def format_ptr(p: typing.Union[str, int, lldb.SBValue]) -> str:
    """ Format the pointer `p` (typically an $rip or $fp/$rbp) for display in stack frames

    Arguments:
        p: Value to format

    Returns:
        Formatted string

    If p is a string, we're just going to assume it's already formatted.
    """
    if isinstance(p, str):
        return p

    if isinstance(p, int):
        pass
    elif isinstance(p, lldb.SBValue):
        p = p.unsigned
    else:
        raise Exception(f"Invalid ptr type {str(type(p))}")

    return f"0x{p:08x}"


#------------------------------------------------------------------------------
# PHP frame info.

def php_filename(func: lldb.SBValue, target: lldb.SBTarget) -> str:
    """ Get the filename where the PHP function is defined

    Arguments:
        func: A HPHP::Func* wrapped in an lldb.SBValue
        target: The current target

    Returns:
        The filename as a string
    """
    m_shared = utils.rawptr(utils.get(func, "m_shared"))
    filename = utils.rawptr(utils.get(m_shared, "m_originalFilename"))

    if filename.unsigned == 0:  # null ptr
        filename = utils.rawptr(utils.get(func, "m_unit", "m_origFilepath"))

    return utils.string_data_val(filename)


def php_line_number(func: lldb.SBValue, pc: int) -> typing.Optional[int]:
    """ Get the line number in the php file associated with the given function and pc

    Uses the shared lineMap if the pc is present.

    Args:
        func: A HPHP::Func* wrapped in an SBValue
        pc: The PC

    Returns:
        line: The line number, if found successfully
    """
    shared = utils.rawptr(utils.get(func, "m_shared"))
    line_map = utils.get(shared, "m_lineMap", "val")

    if line_map.unsigned != 0:
        # TODO test this code path
        i = 0
        while True:
            r = idx.compact_vector_at(line_map, i)
            if r is None:
                break
            first = utils.get(r, "first")
            if utils.get(first, "base").unsigned <= pc and utils.get(first, "past").unsigned > pc:
                return utils.get(r, "second").unsigned
            i += 1

    return php_line_number_from_repo(func, pc)


def php_line_number_from_repo(func, pc):
    """ Get the line number in the php file associated with the given function and pc

    Uses the repo.

    Args:
        func: gdb.Value[HPHP::Func*].
        pc: gdb.Value[HPHP::CompactTaggedPtr<T, uint8_t>::Opaque].

    Returns:
        line: Option[int].
    """
    # TODO implement
    return None


#------------------------------------------------------------------------------
# Frame stringifiers.

def stringify(frame: Frame, fp_len: int=0) -> str:
    """ Stringify a single frame """

    fmt = "#{idx:<2d} {fp:<{fp_len}s} @ {rip}: {func}"
    out = fmt.format(fp_len=fp_len, **frame.__dict__)

    filename = frame.file
    line = frame.line

    if filename is not None:
        out += ' at ' + filename
        if line is not None:
            out += ':' + str(line)

    return out
